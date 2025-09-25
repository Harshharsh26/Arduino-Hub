/*
  Simple Clap Counter with SSD1306 OLED & LM393 Sound Sensor

  🔌 Pin Connections (Arduino UNO):
  --------------------------------
  OLED (I2C)      →   Arduino
  VCC             →   5V
  GND             →   GND
  SDA             →   A4
  SCL             →   A5

  LM393 Sensor    →   Arduino
  VCC             →   5V
  GND             →   GND
  D0              →   D2   (Digital Out, HIGH when clap detected)
*/

#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

// OLED setup
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET    -1
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// Pins
const int SOUND_PIN = 2;   // D0 from LM393 → D2

// State
unsigned long clapCount = 0;
int lastState = LOW;

void setup() {
  pinMode(SOUND_PIN, INPUT);
  Serial.begin(9600);

  // OLED init
  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println(F("SSD1306 init failed"));
    for (;;); // stop here if OLED not found
  }

  display.clearDisplay();
  display.setTextSize(2);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0,0);
  display.println("Clap Counter");
  display.display();
  delay(1000);
  updateDisplay();
}

void loop() {
  int state = digitalRead(SOUND_PIN);

  // Detect rising edge (LOW → HIGH = clap)
  if (state == HIGH && lastState == LOW) {
    clapCount++;
    Serial.print("Clap #");
    Serial.println(clapCount);
    updateDisplay();
    delay(200); // small delay to avoid double counting
  }

  lastState = state;
}

void updateDisplay() {
  display.clearDisplay();
  display.setTextSize(2);
  display.setCursor(0, 0);
  display.println("Claps:");
  display.setTextSize(4);
  display.setCursor(0, 30);
  display.println(clapCount);
  display.display();
}
