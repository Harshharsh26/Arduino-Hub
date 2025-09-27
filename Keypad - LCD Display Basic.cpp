/*
  4x4 Keypad -> SSD1306 OLED + Buzzer Beep
  - Shows the last key pressed on OLED
  - Makes a short beep on each key press

  Wiring (Arduino UNO example):
    SSD1306 (I2C)
      VCC -> 5V
      GND -> GND
      SDA -> A4
      SCL -> A5

    4x4 Keypad
      Rows -> D9, D8, D7, D6
      Cols -> D5, D4, D3, D2

    Buzzer (Piezo, active/passive)
      + (long leg) -> D10
      - (short leg) -> GND
*/

#include <Wire.h>
#include <Keypad.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

// ---------- OLED setup ----------
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET    -1
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

byte rowPins[ROWS] = {9, 8, 7, 6};
byte colPins[COLS] = {5, 4, 3, 2};

Keypad keypad = Keypad(makeKeymap(keys), rowPins, colPins, ROWS, COLS);

// ---------- Buzzer setup ----------
const int BUZZER_PIN = 10;  // digital pin connected to buzzer

// ---------- App state ----------
char lastKey = 0; // stores last pressed key

void setup() {
  Serial.begin(9600);

  // Init OLED
  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println(F("SSD1306 init failed"));
    for (;;); // stop if OLED not found
  }
  display.clearDisplay();
  display.setTextColor(SSD1306_WHITE);

  // Init buzzer pin
  pinMode(BUZZER_PIN, OUTPUT);

  // Show startup message
  display.setTextSize(1);
  display.setCursor(0, 0);
  display.println("4x4 Keypad + Beep");
  display.setCursor(0, 18);
  display.println("Press any key...");
  display.display();
  delay(800);

  showLastKey();
}

void loop() {
  char k = keypad.getKey();
  if (k) {
    lastKey = k;
    Serial.print("Key pressed: ");
    Serial.println(k);

    beep();          // make beep sound
    showLastKey();   // update display
  }
}

// Draw the last pressed key on OLED
void showLastKey() {
  display.clearDisplay();

  display.setTextSize(1);
  display.setCursor(0, 0);
  display.println("Last Key:");

  display.setTextSize(6);
  display.setCursor(28, 18);
  if (lastKey != 0) {
    display.print(lastKey);
  } else {
    display.print("-");
  }

  display.display();
}

// Play a short beep on buzzer
void beep() {
  tone(BUZZER_PIN, 1000, 150); // 1000 Hz for 150 ms
  delay(160);                  // wait until beep finishes
  noTone(BUZZER_PIN);          // stop tone (safety)
}
