# AuraSense: Interactive IoT Lighting System

### View the final version of the code: **[Source Code](https://github.com/CynthiaZHANGovo/CE_IoT/tree/main/AuraSense)**


## Project Overview

AuraSense is an interactive IoT that translates physical presence and movement into a dynamic, colored light display. Using an ultrasonic sensor, it detects a user's distance, speed and direction of movement, expressing this data through a spectrum of colors and animations. The system communicates via MQTT, making it real-time and responsive.

*Inspired by the Science Museum's experience :)*


**Core Concept:** The light's "emotional state" changes based on interaction:
*   **Curiosity & Approach:** The color palette shifts from cool blue to warm red as you get closer, with colors flowing upwards.
*   **Withdrawal & Distance:** The colors cool down, returning to blue as you move away, and flow downwards as the light recedes.
*   **Sudden Movement:** Quick approaches or departures cause the colors to scroll faster and the overall brightness to increase, creating a more "excited" or "startled" response.

This project provides two versions:

| Version | Description |
|---------|--------------|
| **Local Lighting Version** | Uses a NeoPixel LED strip directly connected to the Arduino, basically for testing |
| **Remote (MQTT) Version** | Sends data to the cloud and displays lighting effects on the Lumi web page |

---

## Initial Ideas

### sketch
![My Sketch](https://github.com/CynthiaZHANGovo/CE_IoT/blob/main/VideosAndPhotos/Sketch.jpg)
---

The initial concept explored multiple sensing modalities to create an interactive lighting system. But limited due to the time and devices.

**Considered Sensing Options:**
- **Pulse Detection** ❌ 
  - *Rationale:* While offering personalized biofeedback, required specialized hardware and raised privacy concerns for continuous monitoring
- **Audio Sensing** ❌
  - *Rationale:* Provided ambient environment responsiveness but susceptible to background noise and inconsistent in varied acoustic environments
- **Distance Measurement** ✅ **SELECTED**
  - *Rationale:* Delivers reliable presence detection, simple hardware requirements, and intuitive user interaction through proximity-based control
 

## implementation

### Prototype
*Protoytpe for the whole IoT*

![Prototype Setup](https://github.com/CynthiaZHANGovo/CE_IoT/blob/main/VideosAndPhotos/Prototype.jpg)

### How It Works

1.  **Sensing:** An HC-SR04 ultrasonic sensor continuously measures the distance to an object (like a person) in front of it.
2.  **Processing:** An Arduino MKR WiFi 1010 board processes this data, calculating both the absolute distance and the speed of movement.
3.  **Emotion Mapping:**
    *   **Distance to Color:** A multi-stage gradient maps distance to a color (Red -> Orange -> Yellow -> Green -> Blue).
    *   **Speed to Animation:** The speed of movement controls the scrolling speed of the LED animation and the overall brightness.
    *   **Direction to Animation Flow:** Moving closer triggers a bottom-to-top scroll; moving away triggers a top-to-bottom scroll.
4.  **Communication:** The processed color data is sent as a payload via the MQTT protocol using the PubSubClient library.
5.  **Visualization:** The MQTT message is received by a separate visualization client (like the [Lumi WebSocket Client](https://www.iot.io/projects/lumi/)) to render the colorful, animated display in real-time.
6.  **Local testing:** NeoPixel RGB LED Strip (WS2812), displays physical lighting effects



### Physical Connection
*Actual connection photos*

<p float="left">
  <img src="https://github.com/CynthiaZHANGovo/CE_IoT/blob/main/VideosAndPhotos/board.jpg" width="50%" />
  <img src="https://github.com/CynthiaZHANGovo/CE_IoT/blob/main/VideosAndPhotos/what_it_looks.jpg" width="40%" />
</p>




### Hardware Requirements

*   Arduino MKR WiFi 1010
*   HC-SR04 Ultrasonic Distance Sensor
*   Resistances
*   Wires
*   Breadboard
*   Micro_B-USB Cable


### Software & Library Dependencies

*   [Arduino IDE](https://www.arduino.cc/en/software)
*   Arduino Libraries:
    *   `WiFiNINA`
    *   `PubSubClient`
    *   `SPI`
*   An MQTT Broker (e.g., `mqtt.cetools.org`)
*   A Visualization Client (for mine, [Lumi](https://www.iot.io/projects/lumi/))

---

## Important Version History

| Version | Status | Description | Links | Demo Video |
|---------|--------|-------------|-------|------------|
| **v1.0 - Distance Ranges** | 📜 Archived | roughly distinguish several distance ranges | [Source Code](https://github.com/CynthiaZHANGovo/CE_IoT/compare/ad0f12b0d66cdc71687effed06de716ef45863a2...986309f521e14d347f0f91d1cd565a8aa7c90453) | [Video](https://github.com/CynthiaZHANGovo/CE_IoT/blob/main/VideosAndPhotos/v1.mp4) |
| **v2.0 - Fixed Problem** | 📜 Archived | solved problems in v1, but color changed suddenly | [Source Code](https://github.com/CynthiaZHANGovo/CE_IoT/compare/986309f521e14d347f0f91d1cd565a8aa7c90453...b122b0f2186ee7731a15b881799faed2ada4ab79) | |
| **v3.1 - ColorLineChangedOnebyOne** | 📜 Archived | This version makes the color changing achieved one line by one line, therefore it becomes more smooth. | [Source Code](https://github.com/CynthiaZHANGovo/CE_IoT/compare/b122b0f2186ee7731a15b881799faed2ada4ab79...277526f0186fbcb261938e73845e274e22193348) | [Video](https://github.com/CynthiaZHANGovo/CE_IoT/blob/main/VideosAndPhotos/v3.mp4) |
| **v4.1 - speed-based brightness control** | 📜 Archived | Faster movements create brighter colors while maintaining the row-scrolling animation effect. | [Source Code](https://github.com/CynthiaZHANGovo/CE_IoT/compare/a61a09bc0e654fb48827a1caadbaad0c7560ac1d...bd5d6d186428b23f25da42fd3b5e6d04d364bc92) | [Video](https://github.com/CynthiaZHANGovo/CE_IoT/blob/main/VideosAndPhotos/v4.mp4) |
| **v5.1 — Final version** | 🔥 Current | Added direction-based scrolling, enhanced the color gradient, and improved the brightness gradient for speed-responsive lighting. | [Source Code](https://github.com/CynthiaZHANGovo/CE_IoT/tree/main/distance_vespera) | [Video](https://github.com/CynthiaZHANGovo/CE_IoT/blob/main/VideosAndPhotos/v——final.mp4) | |


---

## Testing
Here are the testing, as well as the testing reflection: 
[Testing](https://github.com/CynthiaZHANGovo/CE_IoT/tree/main/Testing)

## Additional Notes

- **divided_voltage_Note:**  
  - [How I achieved it with added resistances](https://github.com/CynthiaZHANGovo/CE_IoT/blob/main/Document/divided_voltage_Note)


- **Notes in class:**  
  - [see files](https://github.com/CynthiaZHANGovo/CE_IoT/tree/main/Document/CE_Notes-main/Connected%20Evironments)  

---

