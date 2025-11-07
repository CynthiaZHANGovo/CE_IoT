#include <Adafruit_NeoPixel.h>

// ======== NeoPixel Settings ========
#define NEOPIXEL_PIN 6      // LED data pin
#define NUMPIXELS 6         // Number of LEDs for local testing
Adafruit_NeoPixel pixels(NUMPIXELS, NEOPIXEL_PIN, NEO_GRB + NEO_KHZ800);

// ======== HC-SR04 Ultrasonic Sensor ========
#define TRIG_PIN 8
#define ECHO_PIN 9

// ======== Global Variables ========
long lastDistance = 0;           // Last measured distance
int currentR = 0, currentG = 0, currentB = 255;  // Initial color blue
int baseBrightness = 2;        // Initial brightness, default for LED strip
int maxBrightness = 5;         // Maximum brightness for large distance changes

// ======== Function Declarations ========
long readDistanceCM();                                   // Distance measurement
void adaptiveFlashColorLocal(int newR, int newG, int newB, int delta); // Adaptive speed + brightness
void setOverallBrightness(int brightness);              // Set overall brightness

// ======== Setup Initialization ========
void setup() {
  Serial.begin(115200);

  pixels.begin();                     // Initialize LEDs
  setOverallBrightness(baseBrightness); // Set initial brightness
  pixels.show();                      // Display initial state

  pinMode(TRIG_PIN, OUTPUT);          // Ultrasonic trigger pin
  pinMode(ECHO_PIN, INPUT);           // Ultrasonic echo pin

  Serial.println("✅ Local test initialization completed");
}

// ======== Main Loop ========
void loop() {
  long distance = readDistanceCM();   // Read distance
  if (distance <= 0) return;          // Skip invalid readings

  // Limit distance to 5~100 cm, beyond range stays blue
  bool outOfRange = false;
  if (distance < 5 || distance > 80) {
    distance = 100;  // Default blue for farthest distance
    outOfRange = true;
  }

  int newR, newG, newB;

  if (outOfRange) {
    // Stay blue when out of range
    newR = 0;
    newG = 0;
    newB = 255;
  } else {
    // Near→red, Far→blue
    if (distance < 20) {          // Very close
      newR = 255; 
      newG = map(distance, 5, 20, 0, 100); // Gradually increase green
      newB = 0;
    } else if (distance < 50) {   // Close
      newR = map(distance, 20, 40, 255, 0); // Red gradually decreases
      newG = map(distance, 20, 40, 100, 0); // Green gradually decreases
      newB = map(distance, 20, 40, 0, 100); // Blue gradually increases
    } else if (distance < 80) {   // Far
      newR = 0;
      newG = map(distance, 40, 60, 0, 50); // Add some green
      newB = map(distance, 40, 60, 100, 200); // Blue increases
    } else {                       // Very far
      newR = 0;
      newG = 0;
      newB = 255;                   // Farthest blue
    }
  }

  int delta = abs(distance - lastDistance); // Distance change amount

  if (delta > 1) {
    // Significant distance change, call adaptive function
    adaptiveFlashColorLocal(newR, newG, newB, delta);
    currentR = newR; 
    currentG = newG; 
    currentB = newB;
  } else {
    // Minimal distance change, maintain initial brightness without increase
    pixels.setBrightness(baseBrightness);
    // Update color directly but maintain baseBrightness
    for (int i = 0; i < NUMPIXELS; i++) {
      pixels.setPixelColor(i, pixels.Color(newR, newG, newB));
    }
    pixels.show();
  }

  lastDistance = distance;
  delay(100); // 100ms loop delay
}

// ======== HC-SR04 Distance Measurement ========
long readDistanceCM() {
  digitalWrite(TRIG_PIN, LOW);
  delayMicroseconds(2);
  digitalWrite(TRIG_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG_PIN, LOW);

  long duration = pulseIn(ECHO_PIN, HIGH, 30000); // Max 30ms
  if (duration == 0) {
    Serial.println("⚠️ Invalid distance");
    return -1;
  }

  long distance = duration / 58.2; // Convert to centimeters
  Serial.print("📏 Current distance: ");
  Serial.print(distance);
  Serial.println(" cm");
  return distance;
}

// ======== Adaptive Speed and Brightness Change ========
void adaptiveFlashColorLocal(int newR, int newG, int newB, int delta) {
  // Adjust LED sweep speed based on distance change
  int stepDelay;
  if (delta < 3) stepDelay = 120;   // Small change = slow
  else if (delta < 15) stepDelay = 60; // Medium
  else stepDelay = 15;               // Large change = fast

  // Adjust brightness based on delta: large change→bright, small change→dim
  int dynamicBrightness = map(delta, 1, 30, baseBrightness, maxBrightness);
  dynamicBrightness = constrain(dynamicBrightness, baseBrightness, maxBrightness);
  pixels.setBrightness(dynamicBrightness);

  Serial.print("Δ=");
  Serial.print(delta);
  Serial.print("  Speed=");
  Serial.print(stepDelay);
  Serial.print("ms  Brightness=");
  Serial.println(dynamicBrightness);

  // Change colors sequentially from bottom to top
  for (int i = 0; i < NUMPIXELS; i++) {
    pixels.setPixelColor(i, pixels.Color(newR, newG, newB));
    pixels.show();
    delay(stepDelay);
  }
}

// ======== Set Overall Brightness ========
void setOverallBrightness(int brightness) {
  // brightness: 0~255
  pixels.setBrightness(brightness);
  Serial.print("💡 Overall brightness set to: ");
  Serial.println(brightness);
}
