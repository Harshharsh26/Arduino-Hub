/*
  4x4 Keypad -> SSD1306 OLED (show last key pressed)
  - Simple: display only the most recent key press
  - Libraries required:
      Keypad.h
      Adafruit_GFX.h
      Adafruit_SSD1306.h

  Wiring (Arduino UNO example):
    SSD1306 (I2C)
      VCC -> 5V
      GND -> GND
      SDA -> A4
      SCL -> A5

    4x4 Keypad
      Rows -> D9, D8, D7, D6
      Cols -> D5, D4, D3, D2
*/

#include <Wire.h>
#include <Keypad.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

// ---------- OLED setup ----------
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET    -1   // -1 if not used
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// ---------- Keypad setup ----------
const byte ROWS = 4;
const byte COLS = 4;
char keys[ROWS][COLS] = {
  {'1','2','3','A'},
  {'4','5','6','B'},
  {'7','8','9','C'},
  {'*','0','#','D'}
};

// change pins below if needed
byte rowPins[ROWS] = {9, 8, 7, 6};
byte colPins[COLS] = {5, 4, 3, 2};

Keypad keypad = Keypad( makeKeymap(keys), rowPins, colPins, ROWS, COLS );

// ---------- App state ----------
char lastKey = 0; // stores last pressed key

void setup() {
  Serial.begin(9600);

  // Init OLED
  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) { // common I2C address 0x3C
    Serial.println(F("SSD1306 init failed"));
    for (;;); // halt if OLED not found
  }
  display.clearDisplay();
  display.setTextColor(SSD1306_WHITE);

  // show startup message briefly
  display.setTextSize(1);
  display.setCursor(0, 0);
  display.println("4x4 Keypad -> OLED");
  display.setTextSize(1);
  display.setCursor(0, 18);
  display.println("Press any key...");
  display.display();
  delay(800);

  // show initial empty screen
  showLastKey();
}

void loop() {
  char k = keypad.getKey(); // non-blocking; returns 0 if no key
  if (k) {
    lastKey = k;
    Serial.print("Key pressed: ");
    Serial.println(k);
    showLastKey();
  }
}

// Draw the last pressed key on the OLED
void showLastKey() {
  display.clearDisplay();

  display.setTextSize(1);
  display.setCursor(0, 0);
  display.println("Last Key:");

  display.setTextSize(6);           // big character
  display.setCursor(28, 18);        // center-ish
  if (lastKey != 0) {
    display.print(lastKey);
  } else {
    display.print("-");             // nothing pressed yet
  }

  display.display();
}
