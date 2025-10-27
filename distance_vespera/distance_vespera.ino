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

// ======== MQTT Topic ========
String lightId = "29";
String mqtt_topic = "student/CASA0014/luminaire/" + lightId;

// ======== HC-SR04 Ultrasonic Sensor ========
#define TRIG_PIN 8
#define ECHO_PIN 9

// ======== Global Variables ========
long lastDistance = 0;
long lastTime = 0;
const int num_leds = 72; // 6 rows × 12 columns
const int payload_size = num_leds * 3;
byte RGBpayload[payload_size];

// Current color storage
int currentR = 0, currentG = 0, currentB = 255; // Initial blue

// ======== Function Declarations ========
void startWifi();
void reconnectMQTT();
void callback(char* topic, byte* payload, unsigned int length);
long readDistanceCM();
void fillRowColor(int row, int r, int g, int b);
void publishPayload();
void scrollRowsBottomToTop(int newR, int newG, int newB, int delta);
void getGradientColor(long distance, int &r, int &g, int &b);

// ======== Setup Initialization ========
void setup() {
  Serial.begin(115200);
  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);

  startWifi();
  mqttClient.setServer(mqtt_server, mqtt_port);
  mqttClient.setCallback(callback);
  mqttClient.setBufferSize(2000);

  // Initial all blue
  for (int i = 0; i < num_leds; i++) {
    RGBpayload[i * 3 + 0] = 0;
    RGBpayload[i * 3 + 1] = 0;
    RGBpayload[i * 3 + 2] = 255;
  }
  publishPayload();
  lastTime = millis();
  Serial.println(" Setup complete.");
}

// ======== Main Loop ========
void loop() {
  if (!mqttClient.connected()) reconnectMQTT();
  mqttClient.loop();

  long currentTime = millis();
  long distance = readDistanceCM();
  if (distance <= 0) return; // Invalid reading

  // Calculate speed (cm/s)
  float speed = 0;
  if (lastTime > 0) {
    long timeDiff = currentTime - lastTime;
    if (timeDiff > 0) {
      speed = abs(distance - lastDistance) / (float)timeDiff * 1000.0;
    }
  }

  // Limit range 5~100cm
  bool outOfRange = false;
  if (distance < 1 || distance > 500) {
    distance = constrain(distance, 5, 100);
    outOfRange = true;
  }

  int newR, newG, newB;
  getGradientColor(distance, newR, newG, newB);

  int delta = abs(distance - lastDistance);

  // Apply speed-based brightness to all colors
  float brightnessFactor = 1.0;
  if (lastTime > 0) {
    // Map speed to brightness: 0-10 cm/s -> 0.3-1.0 brightness
    brightnessFactor = constrain(map(speed, 0, 20, 0.3, 1.0), 0.3, 1.0);
    
    newR = (int)(newR * brightnessFactor);
    newG = (int)(newG * brightnessFactor);
    newB = (int)(newB * brightnessFactor);
  }

if (delta > 1) {
  // Determine movement direction
  bool movingCloser = (distance < lastDistance);

  if (movingCloser) {
    // Closer: scroll bottom → top (existing behavior)
    scrollRowsBottomToTop(newR, newG, newB, delta);
  } else {
    // Farther: scroll top → bottom
    for (int row = 0; row <= 5; row++) { // reverse direction
      fillRowColor(row, newR, newG, newB);
      publishPayload();
      delay(30); // small delay; you can adjust or reuse your stepDelay logic
    }
    Serial.println("Scroll top→bottom (moving farther)");
  }

    // Keep the adjusted brightness colors
    currentR = newR;
    currentG = newG;
    currentB = newB;
    
    // Fill all LEDs with adjusted brightness colors
    for (int i = 0; i < num_leds; i++) {
      RGBpayload[i * 3 + 0] = currentR;
      RGBpayload[i * 3 + 1] = currentG;
      RGBpayload[i * 3 + 2] = currentB;
    }
    publishPayload();
  }

  lastDistance = distance;
  lastTime = currentTime;

  Serial.print(" Distance: ");
  Serial.print(distance);
  Serial.print("cm  Speed: ");
  Serial.print(speed);
  Serial.print("cm/s  Brightness: ");
  Serial.print(brightnessFactor * 100);
  Serial.print("%  R=");
  Serial.print(newR);
  Serial.print(" G=");
  Serial.print(newG);
  Serial.print(" B=");
  Serial.println(newB);

  delay(30);
}

// ======== Distance → Multi-color Gradient Mapping ========
void getGradientColor(long distance, int &r, int &g, int &b) {
  if (distance < 10) {            // Red
    r = 255; g = 0; b = 0;
  } else if (distance < 15) {     // Orange
    r = 255; g = 100; b = 0;
  } else if (distance < 20) {     // Yellow
    r = 255; g = 255; b = 0;
  } else if (distance < 25) {     // Green
    r = 0; g = 255; b = 0;
  } else if (distance < 40) {     // Blue
    r = 0; g = 0; b = 255;
  }
  else {                        // Blue
    r = 0; g = 0; b = 255;
  }
}

// ======== Scroll Rows Bottom to Top ========
void scrollRowsBottomToTop(int newR, int newG, int newB, int delta) {
  // Reduced delay values for faster scrolling
  int stepDelay;
  if (delta < 3) stepDelay = 120;   // was 250
  else if (delta < 10) stepDelay = 60;  // was 120
  else stepDelay = 15;              // was 40

  for (int row = 5; row >= 0; row--) {
    fillRowColor(row, newR, newG, newB);
    publishPayload();
    delay(stepDelay);
  }

  Serial.print("Fast Scroll bottom→top, delay=");
  Serial.println(stepDelay);
}

// ======== Fill One Row (12 LEDs) with Color ========
void fillRowColor(int row, int r, int g, int b) {
  for (int i = 0; i < 12; i++) {
    int index = i * 6 + row; // 6 LEDs per column
    RGBpayload[index * 3 + 0] = r;
    RGBpayload[index * 3 + 1] = g;
    RGBpayload[index * 3 + 2] = b;
  }
}

// ======== MQTT Publish ========
void publishPayload() {
  mqttClient.publish(mqtt_topic.c_str(), RGBpayload, payload_size);
}

// ======== Distance Measurement ========
long readDistanceCM() {
  digitalWrite(TRIG_PIN, LOW);
  delayMicroseconds(2);
  digitalWrite(TRIG_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG_PIN, LOW);

  // reduce timeout from 30ms -> 10ms for faster response
  long duration = pulseIn(ECHO_PIN, HIGH, 10000);
  if (duration == 0) return -1;

  return duration / 58.2;
}


// ======== Network Functions ========
void startWifi() {
  Serial.print("Connecting to ");
  Serial.println(ssid);
  while (WiFi.begin(ssid, password) != WL_CONNECTED) {
    delay(2000);
    Serial.print(".");
  }
  Serial.println("\n WiFi connected.");
}

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

void callback(char* topic, byte* payload, unsigned int length) {}