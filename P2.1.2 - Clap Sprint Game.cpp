/* Clap Timer Challenge - count claps in 10 seconds
Goal: Start a 10-second countdown; kids clap as many times as they can before time runs out. Final score appears.
Explanation: A clap starts a 10s timer. Each detected clap increments the score shown on the OLED. After 10s the final score displays.
Wiring:
  OLED (I2C): VCC->5V, GND->GND, SDA->A4, SCL->A5
  LM393: VCC->5V, GND->GND, D0->D2
*/

#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET -1
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

const int SOUND_PIN = 2;
unsigned long clapCount = 0;
int lastState = LOW;

const unsigned long countdownMs = 10000; // 10 seconds
bool running = false;
unsigned long startMillis = 0;

void setup() {
  pinMode(SOUND_PIN, INPUT);
  Serial.begin(9600);
  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);
  display.clearDisplay();    // clear screen
  display.setTextSize(2);    // text size 1–3
  display.setTextColor(SSD1306_WHITE); 
  display.setCursor(0,0);    
  display.print("Start");
  display.display();         // update display
  showStartScreen();
}

void loop() {
  int s = digitalRead(SOUND_PIN);

  // start the challenge when a clap is heard and not running
  if (!running && s == HIGH && lastState == LOW) {
    running = true;
    clapCount = 0;
    startMillis = millis();
    showRunning();
    delay(150);
  }

  if (running) {
    if (s == HIGH && lastState == LOW) {
      clapCount++;
      showRunning();
      delay(120);
    }
    if (millis() - startMillis >= countdownMs) {
      running = false;
      showResult();
      delay(3000);
      showStartScreen();
    }
  }

  lastState = s;
}

void showStartScreen() {
  display.clearDisplay();
  display.setTextSize(2);
  display.setCursor(0,8);
  display.println("Clap Sprint");
  display.setTextSize(1);
  display.setCursor(0,40);
  display.println("Clap to start 10s timer!");
  display.display();
}

void showRunning() {
  unsigned long left = (countdownMs - (millis() - startMillis)) / 1000;
  display.clearDisplay();
  display.setTextSize(1);
  display.setCursor(0,0);
  display.print("Time left: ");
  display.print(left);
  display.println(" s");
  display.setTextSize(3);
  display.setCursor(0,20);
  display.print(clapCount);
  display.display();
}

void showResult() {
  display.clearDisplay();
  display.setTextSize(2);
  display.setCursor(0,10);
  display.println("Time's up!");
  display.setTextSize(2);
  display.setCursor(0,35);
  display.print("Score:");
  display.print(clapCount);
  display.display();
}
