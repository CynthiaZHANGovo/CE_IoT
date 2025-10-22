// Random NeoPixel example for Arduino MKR1010
#include <Adafruit_NeoPixel.h>

#define NEOPIXEL_PIN 6
#define NUMPIXELS 8

int red = 0;
int green = 0;
int blue = 0;

// Adafruit_NeoPixel(NUMPIXELS, PIN, NEO_GRB + NEO_KHZ800)
Adafruit_NeoPixel pixels = Adafruit_NeoPixel(NUMPIXELS, NEOPIXEL_PIN, NEO_GRB + NEO_KHZ800);

void setup() {
  Serial.begin(115200);
  pixels.begin();           // initialize
  pixels.show();            // turn off all pixels
  pixels.setBrightness(50); // 0-255
  randomSeed(analogRead(A0)); // seed randomness, A0 floating is ok
}

void loop() {
  for (int i = 0; i < NUMPIXELS; i++) {
    red = random(0, 6) * 50;    // 0,50,100,150,200,250
    green = random(0, 6) * 50;
    blue = random(0, 6) * 50;
    pixels.setPixelColor(i, red, green, blue);
  }
  pixels.show();
  delay(300);
}
