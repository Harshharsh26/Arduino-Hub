#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

// Create display object (128x64 pixels, I2C)
Adafruit_SSD1306 display(128, 64, &Wire, -1);

void setup() {
  display.begin(SSD1306_SWITCHCAPVCC, 0x3C); // 0x3C is OLED I2C address
  display.clearDisplay();    // clear screen
  display.setTextSize(1);    // text size 1–3
  display.setTextColor(SSD1306_WHITE); 
  display.setCursor(0,0);    
  display.print("Hello World");
  display.display();         // update display
}

void loop() { }
