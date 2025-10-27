# Lumi — IoT Interactive Lighting System

Lumi is an IoT-based interactive lighting experience that reacts to human proximity and synchronizes lighting effects between physical devices and a web interface. By combining ultrasonic sensing, real-time MQTT communication, and RGB lighting, Lumi creates an immersive, responsive light interaction both locally and remotely.

---

## Introduction

Lumi explores how physical interactions can extend into digital environments through IoT. When a person approaches the sensor, Lumi dynamically adjusts lighting color, brightness, or animation speed—either on a **local LED strip** or on a **remote web interface**.

This project provides two versions:

| Version | Description |
|---------|--------------|
| **Local Lighting Version** | Uses a NeoPixel LED strip directly connected to the Arduino |
| **Remote (MQTT-Only) Version** | Sends data to the cloud and displays lighting effects on the Lumi web page |

---

## Prototype Components

### 1. HC-SR04 Ultrasonic Distance Sensor
**Purpose:** Measures the distance between a person and the lamp  
**How it works:**  
- Emits ultrasonic waves  
- Waves reflect from an object  
- Calculates distance based on the return time  

**In Lumi:**  
Acts as the interactive input that controls lighting color, brightness, and animation speed based on proximity.

---

### 2. MKR WiFi 1010 (Arduino)
**Purpose:** Main microcontroller for the system  

**Key Features:**
- Built-in Wi-Fi module  
- Designed for IoT development  

**In Lumi:**  
- Reads sensor values  
- Runs lighting animation logic  
- Sends data to MQTT over Wi-Fi  

---

### 3. Wi-Fi (WiFiNINA Library)
**Purpose:** Network connectivity  

**Usage:**  
Enables the Arduino to connect to Wi-Fi and stream real-time data to the MQTT server, syncing data to the Lumi webpage.

---

### 4. MQTT Communication (PubSubClient Library)
**Purpose:** Real-time IoT data transfer  

**Flow:**  
`Arduino → MQTT → Lumi Web App`  

**Usage:**  
- Sends lighting data as MQTT payloads  
- Lightweight and ideal for real-time IoT communication  

---

### 5. NeoPixel RGB LED Strip (WS2812) *(Optional – Local Version)*
**Purpose:** Displays physical lighting effects  

**Why NeoPixel:**  
- Individually addressable RGB LEDs  
- Controlled using a single data line  

**Usage:**  
Local-only version to preview lighting effects without needing the web interface.

---

## Design Logic

The system maps distance to lighting behavior:

| Distance | Output Behavior |
|----------|------------------|
| Far | Soft, calm, slow lighting |
| Medium | Color shifts or increased brightness |
| Close | Fast, vivid, reactive animations |

**Processing Logic:**
1. Read distance from ultrasonic sensor  
2. Normalize distance value (e.g., to brightness or speed)  
3. Apply lighting animation logic  
4. Output either to NeoPixel or send to MQTT  

This ensures both physical and web-based displays follow the same interaction logic.

---

## Version History

| Version | Status | Description | Link |
|---------|--------|-------------|-------|
| **v1.0 — Local Standalone Lighting** | ✅ Released | Physical version using MKR WiFi 1010 + NeoPixel for local testing | *(Add link if available)* |
| **v2.0 — MQTT Remote Lighting** | 🔥 Current | Sends data to cloud and visualizes lighting online | https://www.iot.io/projects/lumi/ |

---

## Additional Notes

- Two modes supported:  
  - **Offline Local Mode:** NeoPixel lighting only  
  - **Online IoT Mode:** Syncs to Lumi web page via MQTT  
- Network performance may affect lighting update speed in remote mode  
- Future improvements may include two-way communication (web → device)

---

