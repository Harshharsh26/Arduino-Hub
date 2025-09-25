/* Improved Noise Meter Face
   Shows :) if quiet, >:( if loud
   Uses averaging for better sensitivity
*/

#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET -1
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

const int MIC_A = A0;  // Analog pin from LM393
int noiseLevel = 0;

// Adjust this after testing in your room
const int QUIET_THRESHOLD = 25;  

void setup() {
  Serial.begin(9600);
  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);
  display.clearDisplay();    // clear screen
  display.setTextSize(2);    // text size 1–3
  display.setTextColor(SSD1306_WHITE); 
  display.setCursor(0,0);    
  display.print("STart ");
  display.display();  
}

void loop() {
  // Take multiple samples and average them
  long sum = 0;
  for (int i = 0; i < 50; i++) {
    sum += analogRead(MIC_A);
    delayMicroseconds(50);  // tiny delay between samples
  }
  noiseLevel = sum / 50;  // average of 50 readings

  Serial.println(noiseLevel);

  // Show result on display
  display.clearDisplay();
  display.setTextSize(1);
  display.setCursor(0,0);
  display.print("Noise Level: ");
  display.println(noiseLevel);

  if (noiseLevel < QUIET_THRESHOLD) {
    drawSmiley();
  } else {
    drawAngry();
  }

  display.display();
  delay(200);  // refresh 5 times per second
}

void drawSmiley() {
  display.setTextSize(4);
  display.setCursor(20,20);
  display.print(":)");
  display.setTextSize(1);
  display.setCursor(90,50);
  display.println("Quiet");
}

void drawAngry() {
  display.setTextSize(4);
  display.setCursor(20,20);
  display.print(">:(");
  display.setTextSize(1);
  display.setCursor(80,50);
  display.println("Loud!");
  delayMicroseconds(2000);
}
