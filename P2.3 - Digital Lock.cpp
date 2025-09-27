/*
  Digital Safe (Keypad + OLED + Buzzer + SG90 Servo)
  - 4x4 keypad to enter PIN
  - SSD1306 OLED shows last key & masked PIN entry
  - Buzzer beeps on every key press
  - SG90 servo: locked/unlocked positions
  - PIN is "1234"
  - Press '#' to submit (open if PIN matches)
  - Press '*' to immediately lock (close)

  Wiring (Arduino UNO example):
  --------------------------------
  SSD1306 (I2C)      -> Arduino
    VCC   -> 5V
    GND   -> GND
    SDA   -> A4
    SCL   -> A5

  4x4 Keypad (example pins)
    Rows -> D9, D8, D7, D6
    Cols -> D5, D4, D3, D2

  Buzzer:
    + -> D10
    - -> GND

  SG90 Servo:
    Orange/Yellow (Signal) -> D11
    Red    (VCC)           -> 5V
    Brown  (GND)           -> GND

  Libraries required:
    Keypad.h
    Adafruit_GFX.h
    Adafruit_SSD1306.h
    Servo.h
*/

#include <Wire.h>
#include <Keypad.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Servo.h>

// ---------- OLED setup ----------
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET    -1
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// ---------- Keypad setup ----------
const byte ROWS = 4;
const byte COLS = 4;
char keysArr[ROWS][COLS] = {
  {'1','2','3','A'},
  {'4','5','6','B'},
  {'7','8','9','C'},
  {'*','0','#','D'}
};
byte rowPins[ROWS] = {9, 8, 7, 6};
byte colPins[COLS] = {5, 4, 3, 2};
Keypad keypad = Keypad(makeKeymap(keysArr), rowPins, colPins, ROWS, COLS);

// ---------- Buzzer & Servo ----------
const int BUZZER_PIN = 10;
const int SERVO_PIN  = 11;
Servo lockServo;

// ---------- PIN & state ----------
const String CORRECT_PIN = "1234";  // correct code
String inputBuf = "";               // stores entered digits
char lastKey = 0;                   // last key pressed for display

// Servo positions (degrees) - tune as needed
const int SERVO_LOCKED_POS = 0;     // closed/locked
const int SERVO_UNLOCKED_POS = 90;  // open/unlocked

// Optional: visual timings
const unsigned long STATUS_SHOW_MS = 1200; // how long to show "Unlocked" or "Wrong PIN"

void setup() {
  Serial.begin(9600);

  // Init OLED
  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println(F("SSD1306 init failed"));
    for (;;);
  }
  display.clearDisplay();
  display.setTextColor(SSD1306_WHITE);

  // Init buzzer pin
  pinMode(BUZZER_PIN, OUTPUT);
  digitalWrite(BUZZER_PIN, LOW);

  // Init servo and set to locked position
  lockServo.attach(SERVO_PIN);
  lockServo.write(SERVO_LOCKED_POS); // start locked

  // Show startup message
  display.setTextSize(1);
  display.setCursor(0,0);
  display.println("Digital Safe");
  display.setCursor(0,12);
  display.println("Enter PIN and press #");
  display.display();
  delay(900);
  showStatus(); // draw initial screen
}

void loop() {
  char k = keypad.getKey(); // non-blocking

  if (k) {
    lastKey = k;
    Serial.print("Key: "); Serial.println(k);

    beep(); // sound feedback

    // If '*' pressed -> immediate lock (clear input)
    if (k == '*') {
      inputBuf = "";
      lockServo.write(SERVO_LOCKED_POS);
      showTemporaryMessage("Locked", STATUS_SHOW_MS);
      showStatus();
      return;
    }

    // If '#' pressed -> submit attempt
    if (k == '#') {
      // Check PIN
      if (inputBuf == CORRECT_PIN) {
        lockServo.write(SERVO_UNLOCKED_POS); // open
        showTemporaryMessage("Unlocked!", STATUS_SHOW_MS);
      } else {
        // wrong PIN
        showTemporaryMessage("Wrong PIN", STATUS_SHOW_MS);
      }
      inputBuf = ""; // clear buffer after attempt
      showStatus();
      return;
    }

    // If 'D' used as backspace (optional): remove last character
    if (k == 'D') {
      if (inputBuf.length() > 0) inputBuf.remove(inputBuf.length()-1);
      showStatus();
      return;
    }

    // Otherwise if numeric or letter keys, append (keep only digits for PIN)
    if ( (k >= '0' && k <= '9') || (k >= 'A' && k <= 'D') ) {
      // For PIN entry we usually want digits; ignore A-D unless you want them as part of PIN
      if (inputBuf.length() < 8) { // limit length so it doesn't overflow display
        inputBuf += k;
      }
      showStatus();
    }
  }
}

// ---------- helper functions ----------

// Simple beep for feedback
void beep() {
  tone(BUZZER_PIN, 1000, 120); // 1kHz, 120 ms
  delay(140);                  // small wait so tone plays (short)
  noTone(BUZZER_PIN);
}

// Show main OLED status (last key + masked PIN)
void showStatus() {
  display.clearDisplay();

  display.setTextSize(1);
  display.setCursor(0,0);
  display.println("Digital Safe");

  // show last key pressed
  display.setTextSize(1);
  display.setCursor(0,14);
  display.print("Last Key: ");
  if (lastKey != 0) display.print(lastKey); else display.print("-");

  // show masked PIN input
  display.setTextSize(2);
  display.setCursor(0, 34);
  String masked = "";
  for (unsigned int i = 0; i < inputBuf.length(); i++) masked += '*';
  if (masked.length() == 0) masked = "--"; // show placeholder when empty
  display.print(masked);

  display.display();
}

// Show a temporary message in the center, e.g., "Unlocked!" or "Wrong PIN"
void showTemporaryMessage(const char *msg, unsigned long ms) {
  display.clearDisplay();
  display.setTextSize(2);
  int x = 0;
  int y = 18;
  display.setCursor(x, y);
  display.println(msg);
  display.display();
  delay(ms);
}
