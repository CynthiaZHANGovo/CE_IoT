#include <SPI.h>
#include <WiFiNINA.h>
#include <PubSubClient.h>
#include "arduino_secrets.h"

// ======== Wi-Fi / MQTT Settings ========
const char* ssid          = SECRET_SSID;
const char* password      = SECRET_PASS;
const char* mqtt_username = SECRET_MQTTUSER;
const char* mqtt_password = SECRET_MQTTPASS;
const char* mqtt_server   = "mqtt.cetools.org";
const int mqtt_port       = 1884;

WiFiClient wifiClient;
PubSubClient mqttClient(wifiClient);

// MQTT Topic
String lightId = "29";
String mqtt_topic = "student/CASA0014/luminaire/" + lightId;

// ======== HC-SR04 Ultrasonic Sensor ========
#define TRIG_PIN 8
#define ECHO_PIN 9

// ======== Global Variables ========
long lastDistance = 0;   // Last measured distance
const int num_leds = 72; // Number of LEDs in remote strip
const int payload_size = num_leds * 3;
byte RGBpayload[payload_size];

// ======== Function Declarations ========
void startWifi();
void reconnectMQTT();
void callback(char* topic, byte* payload, unsigned int length);
long readDistanceCM();
void fillColorToPayload(int r, int g, int b);

// ======== Setup Initialization ========
void setup() {
  Serial.begin(115200);

  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);

  startWifi();
  mqttClient.setServer(mqtt_server, mqtt_port);
  mqttClient.setCallback(callback);
  mqttClient.setBufferSize(2000);

  Serial.println("✅ Setup complete.");
}

// ======== Main Loop ========
void loop() {
  if (!mqttClient.connected()) reconnectMQTT();
  mqttClient.loop();

  long distance = readDistanceCM();
  if (distance <= 0) return; // Invalid reading

  // Limit distance range 5~100cm, beyond remains blue
  bool outOfRange = false;
  if (distance < 5 || distance > 100) {
    distance = 100; // Blue
    outOfRange = true;
  }

  int newR, newG, newB;
  if (outOfRange) {
    newR = 0; newG = 0; newB = 255; // Out of range blue
  } else {
    // Simple red to blue gradient based on distance
    // Close distance = red (255, 0, 0)
    // Far distance = blue (0, 0, 255)
    newR = map(distance, 5, 100, 255, 0);
    newG = 0; 
    newB = map(distance, 5, 100, 0, 255);
  }

  // Update MQTT payload
  fillColorToPayload(newR, newG, newB);
  mqttClient.publish(mqtt_topic.c_str(), RGBpayload, payload_size);

  lastDistance = distance;

  Serial.print("📏 Distance: ");
  Serial.print(distance);
  Serial.print(" cm  🎨 Color: R=");
  Serial.print(newR);
  Serial.print(" G=");
  Serial.print(newG);
  Serial.print(" B=");
  Serial.println(newB);

  delay(100);
}

// ======== HC-SR04 Distance Reading ========
long readDistanceCM() {
  digitalWrite(TRIG_PIN, LOW);
  delayMicroseconds(2);
  digitalWrite(TRIG_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG_PIN, LOW);

  long duration = pulseIn(ECHO_PIN, HIGH, 30000); // Max 30ms
  if (duration == 0) {
    Serial.println("⚠️ Invalid distance reading");
    return -1;
  }
  long distance = duration / 58.2;
  return distance;
}

// ======== Fill MQTT Payload ========
void fillColorToPayload(int r, int g, int b) {
  for (int p = 0; p < num_leds; p++) {
    RGBpayload[p * 3 + 0] = r;
    RGBpayload[p * 3 + 1] = g;
    RGBpayload[p * 3 + 2] = b;
  }
}

// ======== Wi-Fi ========
void startWifi() {
  Serial.print("Connecting to ");
  Serial.println(ssid);
  while (WiFi.begin(ssid, password) != WL_CONNECTED) {
    delay(2000);
    Serial.print(".");
  }
  Serial.println("\n🌐 WiFi connected.");
}

// ======== MQTT Reconnection ========
void reconnectMQTT() {
  while (!mqttClient.connected()) {
    Serial.print("Attempting MQTT connection...");
    if (mqttClient.connect("mkr1010Client", mqtt_username, mqtt_password)) {
      Serial.println("connected!");
    } else {
      Serial.print("failed, rc=");
      Serial.print(mqttClient.state());
      delay(2000);
    }
  }
}

// ======== MQTT Callback ========
void callback(char* topic, byte* payload, unsigned int length) {}