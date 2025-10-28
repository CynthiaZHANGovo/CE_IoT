// Duncan Wilson Oct 2025 - v1 - MQTT messager to Vespera
// Works with MKR1010

#include <SPI.h>
#include <WiFiNINA.h>
#include <PubSubClient.h>
#include "arduino_secrets.h" 
#include <utility/wifi_drv.h>   // library to drive RGB LED on the MKR1010



//  network / MQTT details from arduino_secrets.h
const char* ssid          = SECRET_SSID;       
const char* password      = SECRET_PASS;       
const char* mqtt_username = SECRET_MQTTUSER;   
const char* mqtt_password = SECRET_MQTTPASS;   

// Broker info as in your .ino
const char* mqtt_server   = "mqtt.cetools.org";
const int mqtt_port       = 1884; 

// =======================================================================================

// create wifi object and mqtt object
WiFiClient wifiClient;
PubSubClient mqttClient(wifiClient);

// Make sure to update your lightid value below with the one you have been allocated
String lightId = "29"; // the topic id number or user number being used.

// Here we define the MQTT topic we will be publishing data to
String mqtt_topic = "student/CASA0014/luminaire/" + lightId;            
String clientId = ""; // will set once i have mac address so that it is unique

// NeoPixel / Vespera configuration 
const int num_leds = 72;
const int payload_size = num_leds * 3; // x3 for RGB

// Create the byte array to send in MQTT payload
byte RGBpayload[payload_size];

void setup() {
  Serial.begin(115200);
  //while (!Serial); // optional: wait for serial for debugging
  Serial.println("Vespera - MQTT messenger starting");

  // print your MAC address:
  byte mac[6];
  WiFi.macAddress(mac);
  Serial.print("MAC address: ");
  printMacAddress(mac);

  Serial.print("This device is Vespera ");
  Serial.println(lightId);

  // initialize payload to zero (all off)
  for (int i = 0; i < payload_size; i++) {
    RGBpayload[i] = 0;
  }

  // Connect to WiFi
  startWifi();

  // MQTT setup
  mqttClient.setServer(mqtt_server, mqtt_port);
  mqttClient.setBufferSize(2000);
  mqttClient.setCallback(callback);

  Serial.println("Setup complete");
}
 
void loop() {
  // Reconnect if necessary
  if (!mqttClient.connected()) {
    reconnectMQTT();
  }
  
  if (WiFi.status() != WL_CONNECTED){
    startWifi();
  }
  // keep mqtt alive
  mqttClient.loop();

  // Simple demo animation: sweep green along strip
  for (int n = 0; n < num_leds; n++) {
    send_all_off();
    delay(80);
    send_RGB_to_pixel(0, 250, 0, n);
    delay(150);
  }

  delay(1000);
}

// Function to update the R, G, B values of a single LED pixel
// r,g,b: 0-255, pixel: 0..num_leds-1
void send_RGB_to_pixel(int r, int g, int b, int pixel) {
  // Check if the mqttClient is connected before publishing
  if (mqttClient.connected()) {
    if (pixel < 0 || pixel >= num_leds) return;
    RGBpayload[pixel * 3 + 0] = (byte)r; // Red
    RGBpayload[pixel * 3 + 1] = (byte)g; // Green
    RGBpayload[pixel * 3 + 2] = (byte)b; // Blue

    // Publish the byte array (binary payload)
    mqttClient.publish(mqtt_topic.c_str(), RGBpayload, payload_size);
    
    Serial.print("Published whole byte array after updating pixel ");
    Serial.println(pixel);
  } else {
    Serial.println("MQTT client not connected, cannot publish from send_RGB_to_pixel.");
  }
}

void send_all_off() {
  // Check if the mqttClient is connected before publishing
  if (mqttClient.connected()) {
    for (int pixel = 0; pixel < num_leds; pixel++) {
      RGBpayload[pixel * 3 + 0] = (byte)0; // Red
      RGBpayload[pixel * 3 + 1] = (byte)0; // Green
      RGBpayload[pixel * 3 + 2] = (byte)0; // Blue
    }
    mqttClient.publish(mqtt_topic.c_str(), RGBpayload, payload_size);
    Serial.println("Published an all zero (off) byte array.");
  } else {
    Serial.println("MQTT client not connected, cannot publish from send_all_off.");
  }
}

void send_all_random() {
  // Check if the mqttClient is connected before publishing
  if (mqttClient.connected()) {
    for (int pixel = 0; pixel < num_leds; pixel++) {
      RGBpayload[pixel * 3 + 0] = (byte)random(50,256); // Red
      RGBpayload[pixel * 3 + 1] = (byte)random(50,256); // Green
      RGBpayload[pixel * 3 + 2] = (byte)random(50,256); // Blue
    }
    mqttClient.publish(mqtt_topic.c_str(), RGBpayload, payload_size);
    Serial.println("Published an all random byte array.");
  } else {
    Serial.println("MQTT client not connected, cannot publish from send_all_random.");
  }
}

void printMacAddress(byte mac[]) {
  for (int i = 5; i >= 0; i--) {
    if (mac[i] < 16) {
      Serial.print("0");
    }
    Serial.print(mac[i], HEX);
    if (i > 0) {
      Serial.print(":");
    }
  }
  Serial.println();
}

// ------------------ WiFi / MQTT helper functions ------------------

void startWifi() {
  Serial.print("Connecting to WiFi: ");
  Serial.println(ssid);
  // attempt to connect
  WiFi.begin(ssid, password);

  unsigned long start = millis();
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
    // optional timeout and reset behavior could be added here
    if (millis() - start > 30000) { // 30s timeout -> retry
      Serial.println();
      Serial.println("WiFi connect timeout, retrying...");
      WiFi.begin(ssid, password);
      start = millis();
    }
  }
  Serial.println();
  Serial.print("WiFi connected. IP: ");
  Serial.println(WiFi.localIP());
}

void reconnectMQTT() {
  // Try to connect until successful
  while (!mqttClient.connected()) {
    Serial.print("Attempting MQTT connection to ");
    Serial.print(mqtt_server);
    Serial.print(":");
    Serial.println(mqtt_port);

    // Create a unique client ID using MAC or random number
    String macStr = "";
    byte mac[6];
    WiFi.macAddress(mac);
    for (int i = 0; i < 6; i++) {
      if (mac[i] < 16) macStr += "0";
      macStr += String(mac[i], HEX);
    }
    clientId = "MKR1010-" + macStr;

    // Attempt connect (with or without username/password)
    bool connected;
    if (mqtt_username != nullptr && strlen(mqtt_username) > 0) {
      connected = mqttClient.connect(clientId.c_str(), mqtt_username, mqtt_password);
    } else {
      connected = mqttClient.connect(clientId.c_str());
    }

    if (connected) {
      Serial.println("MQTT connected");
      // optionally subscribe to control topics here
      // mqttClient.subscribe("some/topic");
    } else {
      Serial.print("MQTT connect failed, rc=");
      Serial.print(mqttClient.state());
      Serial.println("; retrying in 5s");
      delay(5000);
    }
  }
}

// Simple callback (not used here but required by PubSubClient)
void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] len=");
  Serial.println(length);
  // If you want to parse incoming messages, do it here.
}












