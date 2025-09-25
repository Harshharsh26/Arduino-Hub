/*
  Clap Counter with SSD1306 OLED & LM393 Sound Sensor

  🔌 Pin Connections:

  Arduino UNO   →   SSD1306 OLED (I²C)
  ------------------------------------
  5V           →   VCC
  GND          →   GND
  A4 (SDA)     →   SDA
  A5 (SCL)     →   SCL

  Arduino UNO   →   LM393 Sound Sensor
  ------------------------------------
  5V           →   VCC
  GND          →   GND
  D2           →   D0 (Digital Out)  // detects clap as HIGH pulse
  A0           →   A0 (Analog Out)   // optional, for sound-level bar

  Output:
  - SSD1306 OLED shows total clap count + live sound-level bar
  - Serial Monitor prints debug values
*/

#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

// OLED settings
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET    -1
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// Pins
const uint8_t SOUND_DIGITAL_PIN = 2;   // D0 from LM393 → Arduino D2
const uint8_t SOUND_ANALOG_PIN  = A0;  // A0 from LM393 → Arduino A0

// Debounce / timing
unsigned long lastDebounceTime = 0;
const unsigned long debounceDelay = 40;   // ms debounce for stable input
unsigned long lastClapTime = 0;
const unsigned long clapCooldown = 300;   // ms cooldown to avoid double-counting same clap

// State
int lastSoundState = LOW;
unsigned long clapCount = 0;

// For simple peak display smoothing
int analogPeak = 0;
unsigned long lastAnalogSample = 0;

void setup() {
  // Pins
  pinMode(SOUND_DIGITAL_PIN, INPUT);

  // Serial for debugging
  Serial.begin(9600);
  Serial.println("Clap Counter starting...");

  // OLED init
  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println(F("SSD1306 init failed"));
    for (;;); // stop here if OLED not found
  }
  display.clearDisplay();
  display.setTextColor(SSD1306_WHITE);
  display.setTextSize(1);
  display.setCursor(0,0);
  display.println("Clap Counter");
  display.display();
  delay(700);
  updateDisplay();
}

void loop() {
  unsigned long now = millis();

  // Read digital sound output
  int soundState = digitalRead(SOUND_DIGITAL_PIN);

  // Debounce handling
  if (soundState != lastSoundState) {
    lastDebounceTime = now;
    lastSoundState = soundState;
  }

  if ((now - lastDebounceTime) > debounceDelay) {
    // If a clean rising edge and cooldown passed, count as a clap
    if (soundState == HIGH && (now - lastClapTime) > clapCooldown) {
      clapCount++;
      lastClapTime = now;
      Serial.print("Clap #");
      Serial.println(clapCount);
      updateDisplay();
    }
  }

  // Read analog periodically to draw a little bar (for visual fun)
  if (now - lastAnalogSample > 80) {
    int a = analogRead(SOUND_ANALOG_PIN); // 0 - 1023
    // simple peak smoothing
    if (a > analogPeak) analogPeak = a;
    else analogPeak = max(0, analogPeak - 20); // decay
    lastAnalogSample = now;
    updateSoundBar(analogPeak);
  }
}

// Update whole OLED: title + big clap count
void updateDisplay() {
  display.clearDisplay();

  // Title
  display.setTextSize(1);
  display.setCursor(0,0);
  display.println("Clap Counter");

  // Big count
  display.setTextSize(4);
  display.setCursor(0,12);
  if (clapCount < 10000) {
    display.print(clapCount);
  } else {
    display.print("Lots");
  }

  // Footer
  display.setTextSize(1);
  display.setCursor(0,56);
  display.print("Clap Total: ");
  display.print(clapCount);

  display.display();

  // draw initial sound bar
  updateSoundBar(analogPeak);
}

// Draw a small horizontal sound-level bar (0 - 1023 mapped to 0 - 110)
void updateSoundBar(int level) {
  int width = map(constrain(level, 0, 1023), 0, 1023, 0, 110);

  int x = 8;
  int y = 52;
  int h = 6;

  display.fillRect(x, y, 110, h, SSD1306_BLACK); // clear bar area
  display.drawRect(x-1, y-1, 112, h+2, SSD1306_WHITE); // border
  if (width > 0) {
    display.fillRect(x, y, width, h, SSD1306_WHITE);
  }

  // Label with analog value
  display.setTextSize(1);
  display.setCursor(0,40);
  display.print("Level: ");
  display.print(level);

  display.display();
}
