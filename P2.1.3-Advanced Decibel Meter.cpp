/* Decibel meter (Serial calibration + EEPROM)
   - Mic AO -> A0
   - OLED SSD1306 (I2C) -> SDA A4, SCL A5
   - Serial commands:
       c : start calibration (measure then type phone SPL)
       s : save current calibration to EEPROM
       r : reset/clear calibration
       p : print current calibration value
*/

#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <EEPROM.h>
#include <math.h>

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET -1
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// ----- Hardware pins & sampling -----
const uint8_t MIC_PIN = A0;

// RMS measurement window and sampling
const unsigned long SAMPLE_WINDOW_MS = 120; // measurement window in ms
const unsigned long SAMPLE_RATE = 5000UL;   // approximate samples per second
const unsigned long SAMPLE_PERIOD_US = 1000000UL / SAMPLE_RATE;

// ADC reference voltage (change to 3.3 if you're using 3.3V ADC ref)
const float VREF_VOLTS = 5.0f;

// EEPROM address to store float calibration offset
const int EEPROM_ADDR = 0;

// Calibration offset variable (unique name to avoid macro collisions)
// SPL_estimate = dBFS + CALIB_OFFSET
float CALIB_OFFSET = 0.0f;
bool calibLoaded = false;

// ---- Forward declarations ----
void showHiSplash();
void screenFlashTest();
void printHelp();
void loadCalibration();
void saveCalibration();
float measureVrmsMs(unsigned long windowMs);
void drawMeter(float spl, float dbfs);

void setup() {
  Serial.begin(115200);
  delay(50);
  Serial.println(F("Decibel meter starting... (Serial calibration)"));

  // OLED init (try 0x3C then 0x3D)
  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println(F("[WARN] OLED not found at 0x3C, trying 0x3D..."));
    if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3D)) {
      Serial.println(F("[ERROR] OLED not found at 0x3D either. Halt."));
      for (;;) {}
    } else {
      Serial.println(F("[INFO] OLED initialized at 0x3D"));
    }
  } else {
    Serial.println(F("[INFO] OLED initialized at 0x3C"));
  }

  // quick visual checks
  showHiSplash();
  screenFlashTest();

  // load calibration from EEPROM (if valid)
  loadCalibration();
  if (calibLoaded) {
    Serial.print(F("[INFO] Loaded CALIB_OFFSET = "));
    Serial.println(CALIB_OFFSET, 4);
  } else {
    Serial.println(F("[INFO] No valid calibration in EEPROM. Use 'c' to calibrate."));
  }

  // initial user hint on OLED
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(6, 18);
  display.print("Open Serial @115200");
  display.setCursor(6, 34);
  display.print("Type 'c' to calibrate");
  display.display();
  delay(900);

  printHelp();
}

void loop() {
  // Handle serial commands (non-blocking read)
  if (Serial.available()) {
    String cmd = Serial.readStringUntil('\n');
    cmd.trim();
    if (cmd.length() == 0) { /* ignore empty */ }
    else if (cmd.equalsIgnoreCase("c")) {
      Serial.println(F("\n[CMD] Calibration started..."));
      Serial.println(F("Place phone playing steady tone/noise near mic."));
      // take multiple passes for stability
      const int passes = 3;
      float accVrms = 0.0f;
      for (int i = 0; i < passes; i++) {
        float vr = measureVrmsMs(400); // 400 ms -> ~1.2s total
        accVrms += vr;
        Serial.print(F("  meas Vrms: "));
        Serial.print(vr, 6);
        Serial.println(F(" V"));
      }
      float vrefMeas = accVrms / (float)passes;
      if (vrefMeas < 1e-6f) vrefMeas = 1e-6f; // avoid zeros

      float dbfs_ref = 20.0f * log10(vrefMeas / VREF_VOLTS);
      Serial.print(F("\nMeasured Vrms (avg) = "));
      Serial.print(vrefMeas, 6);
      Serial.println(F(" V"));
      Serial.print(F("Measured dBFS = "));
      Serial.print(dbfs_ref, 3);
      Serial.println(F(" dBFS"));

      Serial.println(F("\nType PHONE app SPL (e.g., 75.5) then Enter, or type 'skip' to cancel:"));
      // wait for user input (blocking is fine here)
      while (!Serial.available()) { delay(10); }
      String sval = Serial.readStringUntil('\n');
      sval.trim();
      if (sval.equalsIgnoreCase("skip")) {
        Serial.println(F("[INFO] Calibration canceled."));
      } else {
        float phoneSPL = sval.toFloat();
        if (phoneSPL == 0.0f && sval != "0" && sval != "0.0") {
          Serial.println(F("[ERROR] Invalid number. Calibration aborted."));
        } else {
          CALIB_OFFSET = phoneSPL - dbfs_ref;
          calibLoaded = true;
          saveCalibration();
          Serial.print(F("[OK] Calibration saved. CALIB_OFFSET = "));
          Serial.println(CALIB_OFFSET, 4);

          // brief OLED feedback
          display.clearDisplay();
          display.setTextSize(1);
          display.setCursor(6, 8);
          display.print("Calibration saved:");
          display.setCursor(6, 28);
          display.print("offset = ");
          display.print(CALIB_OFFSET, 2);
          display.display();
          delay(1400);
        }
      }
      printHelp();
    }
    else if (cmd.equalsIgnoreCase("s")) {
      if (calibLoaded) {
        saveCalibration();
        Serial.println(F("[OK] Calibration saved to EEPROM."));
      } else {
        Serial.println(F("[ERR] No calibration to save."));
      }
      printHelp();
    }
    else if (cmd.equalsIgnoreCase("r")) {
      // clear calibration (mark invalid in EEPROM)
      calibLoaded = false;
      CALIB_OFFSET = 0.0f;
      float nanMark = NAN;
      EEPROM.put(EEPROM_ADDR, nanMark); // write invalid marker
      Serial.println(F("[OK] Calibration cleared from EEPROM."));
      printHelp();
    }
    else if (cmd.equalsIgnoreCase("p")) {
      Serial.print(F("[INFO] CALIB_OFFSET = "));
      if (calibLoaded) Serial.println(CALIB_OFFSET, 4);
      else Serial.println(F("not set"));
      printHelp();
    }
    else {
      Serial.println(F("[ERR] Unknown command."));
      printHelp();
    }
  }

  // Normal measurement
  float vrms = measureVrmsMs(SAMPLE_WINDOW_MS);
  if (vrms < 1e-6f) vrms = 1e-6f;
  float dbfs = 20.0f * log10(vrms / VREF_VOLTS);
  float spl = dbfs + CALIB_OFFSET;

  drawMeter(spl, dbfs);

  // occasional serial log
  static unsigned long lastLog = 0;
  if (millis() - lastLog > 1500) {
    lastLog = millis();
    Serial.print(F("Vrms: "));
    Serial.print(vrms, 6);
    Serial.print(F(" V, dBFS: "));
    Serial.print(dbfs, 2);
    Serial.print(F(" dBFS, SPL: "));
    if (calibLoaded) Serial.println(spl, 2);
    else Serial.println(F("N/A (not calibrated)"));
  }

  delay(80);
}

// ---------- Helper implementations ----------

void showHiSplash() {
  display.clearDisplay();
  display.setTextColor(SSD1306_WHITE);
  display.setTextSize(4);
  int16_t x1, y1; uint16_t w, h;
  display.getTextBounds("Hi", 0, 0, &x1, &y1, &w, &h);
  int16_t cx = (SCREEN_WIDTH - w) / 2;
  int16_t cy = (SCREEN_HEIGHT - h) / 2;
  display.setCursor(cx, cy);
  display.print("Hi");
  display.display();
  delay(2000);
}

void screenFlashTest() {
  display.clearDisplay();
  display.fillRect(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, SSD1306_WHITE);
  display.display();
  delay(250);
  display.clearDisplay();
  display.display();
  delay(120);
}

void printHelp() {
  Serial.println();
  Serial.println(F("Commands:"));
  Serial.println(F("  c  - start serial calibration (measure then enter phone SPL)"));
  Serial.println(F("  s  - save current calibration to EEPROM"));
  Serial.println(F("  r  - reset/clear calibration"));
  Serial.println(F("  p  - print current calibration value"));
  Serial.println();
  Serial.println(F("Calibration flow:"));
  Serial.println(F("  1) On phone app play steady tone/noise and note SPL."));
  Serial.println(F("  2) In Serial type 'c' then Enter."));
  Serial.println(F("  3) After measurement, enter phone SPL value (e.g., 74.5)"));
  Serial.println();
}

void loadCalibration() {
  float f;
  EEPROM.get(EEPROM_ADDR, f);
  // check plausible float
  if (isfinite(f) && f > -200.0f && f < 200.0f) {
    CALIB_OFFSET = f;
    calibLoaded = true;
  } else {
    calibLoaded = false;
  }
}

void saveCalibration() {
  EEPROM.put(EEPROM_ADDR, CALIB_OFFSET);
}

// RMS measurement over a window in ms
float measureVrmsMs(unsigned long windowMs) {
  unsigned long start = millis();
  unsigned long samples = 0;
  double sumSquares = 0.0;

  // quick DC offset estimate (small set)
  long dcSum = 0;
  const uint8_t DC_N = 16;
  for (uint8_t i = 0; i < DC_N; i++) {
    dcSum += analogRead(MIC_PIN);
    delayMicroseconds(200);
  }
  double dc = (double)dcSum / DC_N;

  while (millis() - start < windowMs) {
    unsigned long t0 = micros();
    int raw = analogRead(MIC_PIN);
    double centered = (double)raw - dc;
    sumSquares += centered * centered;
    samples++;

    unsigned long dt = micros() - t0;
    if (dt < SAMPLE_PERIOD_US) delayMicroseconds(SAMPLE_PERIOD_US - dt);
  }

  if (samples == 0) return 0.0f;
  double meanSquare = sumSquares / (double)samples;
  double vrms = sqrt(meanSquare) * (VREF_VOLTS / 1023.0); // ADC units -> volts
  return (float)vrms;
}

void drawMeter(float spl, float dbfs) {
  display.clearDisplay();

  // Title
  display.setTextSize(1);
  display.setCursor(6, 0);
  display.setTextColor(SSD1306_WHITE);
  display.print("dB METER (approx)");

  // Big SPL (if calibrated)
  display.setTextSize(2);
  display.setCursor(6, 14);
  if (calibLoaded) {
    display.print((int)round(spl));
    display.print(" dB");
  } else {
    display.print("--- dB");
  }

  // dBFS small
  display.setTextSize(1);
  display.setCursor(6, 40);
  display.print("dBFS:");
  display.setCursor(48, 40);
  display.print(dbfs, 1);

  // bar meter
  int barX = 6, barY = 52, barW = 116, barH = 8;
  display.drawRoundRect(barX - 1, barY - 1, barW + 2, barH + 2, 3, SSD1306_WHITE);

  // map SPL to bar between 30..120 dB (tweakable)
  float minSPL = 30.0f;
  float maxSPL = 120.0f;
  float pct = (spl - minSPL) / (maxSPL - minSPL);
  if (pct < 0) pct = 0;
  if (pct > 1) pct = 1;
  int fill = (int)(pct * barW);
  if (fill > 0) display.fillRect(barX, barY, fill, barH, SSD1306_WHITE);

  display.display();
}
