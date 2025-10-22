// MKR1010 - MQTT <-> Local NeoPixel sync (first 8 remote pixels -> local 8 LEDs)
// Uses values from arduino_secrets.h

#include <SPI.h>
#include <WiFiNINA.h>
#include <PubSubClient.h>
#include "arduino_secrets.h"
#include <Adafruit_NeoPixel.h>
#include <utility/wifi_drv.h>

//  CONFIG 
#define NEOPIXEL_PIN 6
#define NUMPIXELS 8           // local physical LEDs

// Network / MQTT (from arduino_secrets.h)
const char* ssid          = SECRET_SSID;
const char* password      = SECRET_PASS;
const char* mqtt_username = SECRET_MQTTUSER;
const char* mqtt_password = SECRET_MQTTPASS;

// Broker info
const char* mqtt_server   = "mqtt.cetools.org";
const int   mqtt_port     = 1884;

// light ID
String lightId = "29";

// Vespera config
const int remote_num_leds = 72;
const int remote_payload_size = remote_num_leds * 3; // 72 * 3 = 216

// ------------------------------------------------

WiFiClient wifiClient;
PubSubClient mqttClient(wifiClient);

Adafruit_NeoPixel pixels = Adafruit_NeoPixel(NUMPIXELS, NEOPIXEL_PIN, NEO_GRB + NEO_KHZ800);

String mqtt_topic = "student/CASA0014/luminaire/" + lightId;
String clientId = "";

byte remotePayload[remote_payload_size]; // buffer we publish as remote frame

// ----------- helpers forward decl --------------------
void startWifi();
void reconnectMQTT();
void callback(char* topic, byte* payload, unsigned int length);
void printMacAddress(byte mac[]);

// -------------------- setup --------------------
void setup() {
  Serial.begin(115200);
  Serial.println();
  Serial.println("MQTT <-> Local NeoPixel sync starting...");

  // init local NeoPixel
  pixels.begin();
  pixels.show();
  pixels.setBrightness(80);

  // zero remotePayload initially
  for (int i = 0; i < remote_payload_size; i++) remotePayload[i] = 0;

  // WiFi
  startWifi();

  // MQTT client setup
  mqttClient.setServer(mqtt_server, mqtt_port);
  mqttClient.setCallback(callback);
  mqttClient.setBufferSize(2000);

  // print MAC
  byte mac[6];
  WiFi.macAddress(mac);
  Serial.print("MAC: ");
  printMacAddress(mac);
  Serial.print("This device lightId: ");
  Serial.println(lightId);
}

// -------------------- main loop --------------------
void loop() {
  if (WiFi.status() != WL_CONNECTED) startWifi();
  if (!mqttClient.connected()) reconnectMQTT();
  mqttClient.loop();

  // Example behaviour: publish a sweep (remote) and also update local mapping
  for (int p = 0; p < remote_num_leds; p++) {
    // make a demo color (green sweep)
    int r = 0;
    int g = 200;
    int b = 0;

    // update the remotePayload array (so we publish full frame)
    remotePayload[p * 3 + 0] = (byte)r;
    remotePayload[p * 3 + 1] = (byte)g;
    remotePayload[p * 3 + 2] = (byte)b;

    // publish the whole frame (binary)
    if (mqttClient.connected()) {
      mqttClient.publish(mqtt_topic.c_str(), remotePayload, remote_payload_size);
      Serial.print("Published remote frame with pixel ");
      Serial.println(p);
    }

    // LOCAL: only reflect remote pixels 0..7 to local 0..7
    if (p < NUMPIXELS) {
      pixels.setPixelColor(p, r, g, b); // direct 1:1 mapping for first 8 pixels
      pixels.show();
    }

    delay(120); // animation timing (tweak as needed)
  }

  // pause between passes
  delay(800);
}

// -------------------- send helper functions --------------------
// You can also call these from loop or via serial commands

void send_all_off_remote_and_local() {
  // zero remote buffer
  for (int i = 0; i < remote_payload_size; i++) remotePayload[i] = 0;
  if (mqttClient.connected()) mqttClient.publish(mqtt_topic.c_str(), remotePayload, remote_payload_size);

  // local off
  for (int i = 0; i < NUMPIXELS; i++) pixels.setPixelColor(i, 0, 0, 0);
  pixels.show();
}

void send_all_random_remote_and_local() {
  for (int p = 0; p < remote_num_leds; p++) {
    remotePayload[p * 3 + 0] = (byte)random(50, 256);
    remotePayload[p * 3 + 1] = (byte)random(50, 256);
    remotePayload[p * 3 + 2] = (byte)random(50, 256);
  }
  if (mqttClient.connected()) mqttClient.publish(mqtt_topic.c_str(), remotePayload, remote_payload_size);

  // local: random for first NUMPIXELS
  for (int i = 0; i < NUMPIXELS; i++) {
    pixels.setPixelColor(i, random(50, 256), random(50, 256), random(50, 256));
  }
  pixels.show();
}

// -------------------- MQTT callback --------------------
// If a full frame (72*3 bytes) is published to the same mqtt_topic,
// this callback will pick it up (because we subscribe) and update local first 8 pixels.
void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("MQTT message arrived: ");
  Serial.print(topic);
  Serial.print(" (");
  Serial.print(length);
  Serial.println(" bytes)");

  // If topic matches our publish topic (exact), try to use payload
  if (String(topic) == mqtt_topic) {
    // If payload length >= needed for first 8 pixels (8*3 = 24), update local
    if (length >= (NUMPIXELS * 3)) {
      // payload is binary RGB bytes; copy first 24 bytes (0..23)
      for (int i = 0; i < NUMPIXELS; i++) {
        int idx = i * 3;
        int r = payload[idx + 0];
        int g = payload[idx + 1];
        int b = payload[idx + 2];
        pixels.setPixelColor(i, r, g, b);
      }
      pixels.show();
      Serial.println("Local first 8 pixels updated from incoming remote frame.");
    } else if (length >= 3) {
      // maybe someone published a single pixel (3 bytes) or shorter - handle minimally
      // If it's exactly 3 + pixel index header (not standard), ignore for now
      Serial.println("Incoming payload too short for full mapping; ignoring.");
    } else {
      Serial.println("Incoming payload not understood.");
    }
  } else {
    Serial.println("Message on other topic; ignored.");
  }
}

// -------------------- WiFi & MQTT connect functions --------------------
void startWifi() {
  Serial.print("Connecting to WiFi: ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);
  unsigned long start = millis();
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
    if (millis() - start > 30000) {
      Serial.println("\nWiFi timeout, retrying...");
      WiFi.begin(ssid, password);
      start = millis();
    }
  }
  Serial.print("\nWiFi connected. IP: ");
  Serial.println(WiFi.localIP());
}

void reconnectMQTT() {
  while (!mqttClient.connected()) {
    Serial.print("Connecting to MQTT ");
    Serial.print(mqtt_server);
    Serial.print(":");
    Serial.println(mqtt_port);

    // unique clientId (use MAC)
    byte mac[6];
    WiFi.macAddress(mac);
    String macStr = "";
    for (int i = 0; i < 6; i++) {
      if (mac[i] < 16) macStr += "0";
      macStr += String(mac[i], HEX);
    }
    clientId = "MKR1010-" + macStr;

    bool ok;
    if (mqtt_username != nullptr && strlen(mqtt_username) > 0) {
      ok = mqttClient.connect(clientId.c_str(), mqtt_username, mqtt_password);
    } else {
      ok = mqttClient.connect(clientId.c_str());
    }

    if (ok) {
      Serial.println("MQTT connected.");
      // Subscribe to the same topic so incoming frames (from Lumi or others) are received.
      mqttClient.subscribe(mqtt_topic.c_str());
      Serial.print("Subscribed to topic: ");
      Serial.println(mqtt_topic);
    } else {
      Serial.print("MQTT connect failed, rc=");
      Serial.print(mqttClient.state());
      Serial.println("; retrying in 5s");
      delay(5000);
    }
  }
}

// -------------------- utility --------------------
void printMacAddress(byte mac[]) {
  for (int i = 5; i >= 0; i--) {
    if (mac[i] < 16) Serial.print("0");
    Serial.print(mac[i], HEX);
    if (i > 0) Serial.print(":");
  }
  Serial.println();
}
