/* Simple Noise Face (RMS) - shows :) when quiet, >:( when loud
   - Uses A0 (analog) for RMS measurement
   - SSD1306 OLED (I2C) shows smiley or angry face

   Wiring:
     OLED (I2C): VCC->5V, GND->GND, SDA->A4, SCL->A5
     Mic A0     : A0 -> analog output of mic module (LM393 or better amp)

   Notes:
     - The sketch samples A0 quickly for a short window, computes RMS,
       smooths it, and compares to calibrated quietLevel.
     - If the mic signal is too small, increase module gain (pot) or use a better mic amp.
*/

#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Arduino.h>

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET -1
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// Prototype
void updateDisplay(bool quiet);
void quickCalibrate();

const int MIC_PIN = A0;            // analog input from mic
const unsigned int WINDOW_MS = 20; // RMS window (ms)
const unsigned int SAMPLE_DELAY_US = 200; // microseconds between samples (~5kHz)
const float SMOOTH_ALPHA = 0.18;   // smoothing factor (0..1)

float smoothRms = 0.0;
float quietLevel = 0.0;
bool calibrated = false;

void setup() {
  Serial.begin(115200);
  pinMode(MIC_PIN, INPUT);

  // OLED init
  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println(F("SSD1306 init failed"));
    for (;;);
  }
  display.clearDisplay();
  display.setTextColor(SSD1306_WHITE);

  // Quick auto-calibration to get quiet baseline
  delay(200);
  quickCalibrate();
  updateDisplay(true);
}

void loop() {
  // 1) sample for WINDOW_MS and compute mean and sum of squared deviations
  unsigned long t0 = millis();
  unsigned long tend = t0 + WINDOW_MS;

  // First pass: compute mean (quick)
  long n = 0;
  double acc = 0.0;
  while (millis() < tend) {
    acc += analogRead(MIC_PIN);
    n++;
    delayMicroseconds(SAMPLE_DELAY_US);
  }
  if (n == 0) n = 1;
  double mean = acc / (double)n;

  // Second pass: RMS (sum squares of (sample - mean))
  double ss = 0.0;
  for (long i = 0; i < n; i++) {
    double v = analogRead(MIC_PIN) - mean;
    ss += v * v;
    delayMicroseconds(SAMPLE_DELAY_US);
  }
  double msq = ss / (double)max(1, (int)n);
  double vrms = sqrt(msq); // RMS in ADC units

  // Smooth value for stable display
  smoothRms = (SMOOTH_ALPHA * vrms) + (1.0 - SMOOTH_ALPHA) * smoothRms;

  // Compute relative level above quiet baseline
  quietLevel = 0.15;
  float rel = smoothRms - quietLevel;
  if (rel < 0) rel = 0;

  // Decide quiet vs loud using a simple threshold
  // Pick threshold experimentally; 20 works well for many modules after quickCalibrate
  const float THRESH = 0.40;//20.0;
  bool isQuiet = (rel < THRESH);

  // Update display when state changes (or periodically you can update always)
  static bool lastState = true;
  if (isQuiet != lastState) {
    updateDisplay(isQuiet);
    lastState = isQuiet;
  }

  // Debug (optional)
   Serial.print("vrms: "); Serial.print(vrms,2);
   Serial.print("quietLevel: "); Serial.print(quietLevel,2);
   Serial.print(" smooth: "); Serial.print(smoothRms,2);
   Serial.print(" rel: "); Serial.println(rel,2);

  delay(80); // small pause — controls refresh speed
}

// Simple quick calibration: sample a few times and set quietLevel
void quickCalibrate() {
  const int PASSES = 5;
  double vals[PASSES];
  for (int p = 0; p < PASSES; p++) {
    unsigned long t0 = millis();
    unsigned long tend = t0 + WINDOW_MS;
    long n = 0;
    double acc = 0.0;
    while (millis() < tend) {
      acc += analogRead(MIC_PIN);
      n++;
      delayMicroseconds(SAMPLE_DELAY_US);
    }
    if (n == 0) n = 1;
    double mean = acc / (double)n;

    double ss = 0.0;
    for (int i = 0; i < n; i++) {
      double v = analogRead(MIC_PIN) - mean;
      ss += v * v;
      delayMicroseconds(SAMPLE_DELAY_US);
    }
    double msq = ss / (double)max(1, (int)n);
    double rms = sqrt(msq);
    vals[p] = rms;
    delay(40);
  }
  // sort and pick middle value (median-ish)
  for (int i = 0; i < PASSES - 1; i++) {
    for (int j = i + 1; j < PASSES; j++) {
      if (vals[j] < vals[i]) {
        double t = vals[i]; vals[i] = vals[j]; vals[j] = t;
      }
    }
  }
  quietLevel = vals[PASSES / 2];
  calibrated = true;
  Serial.print("Calibrated quietLevel = ");
  Serial.println(quietLevel, 3);
}

// Display smiley or angry face (simple text-based faces)
void updateDisplay(bool quiet) {
  display.clearDisplay();

  display.setTextSize(1);
  display.setCursor(0,0);
  display.print("Noise Meter");

  display.setTextSize(4);
  display.setCursor(28, 18);
  if (quiet) {
    display.println(":)"); // smiley
    display.setTextSize(1);
    display.setCursor(80, 52);
    display.println("Quiet");
  } else {
    display.println(">:("); // angry
    display.setTextSize(1);
    display.setCursor(80, 52);
    display.println("Loud!");
    delayMicroseconds(2000);
  }
  display.display();
}
