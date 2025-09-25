/* Target Game - Clap exactly the target number
Goal: OLED shows a random target number. Kids must clap exactly that many times. If they match, the screen shows “You Win!” otherwise “Too Many!” after they finish.
Explanation: The game gives 5 seconds to clap. Every rising HIGH on D2 increments clapCount. After time ends, it compares count vs target and shows result. Then it starts a new round.
Wiring:
  OLED (I2C): VCC->5V, GND->GND, SDA->A4, SCL->A5
  LM393: VCC->5V, GND->GND, D0->D2 (digital)
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
unsigned long startTime = 0;
const unsigned long gameDuration = 5000; // 5 seconds to perform claps
bool gameRunning = false;
int target = 0;

void setup() {
  pinMode(SOUND_PIN, INPUT);
  Serial.begin(9600);
  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);
  display.clearDisplay();
  randomSeed(analogRead(A3));
  newGame();
}

void loop() {
  int s = digitalRead(SOUND_PIN);
  if (!gameRunning) return;

  // rising edge = clap
  if (s == HIGH && lastState == LOW) {
    clapCount++;
    updateDisplay();
    delay(150);
  }
  lastState = s;

  if (millis() - startTime >= gameDuration) {
    gameRunning = false;
    showResult();
    delay(2000);
    newGame(); // start new round
  }
}

void newGame() {
  target = random(3, 9); // target between 3 and 8
  clapCount = 0;
  gameRunning = true;
  startTime = millis();
  display.clearDisplay();
  display.setTextSize(1);
  display.setCursor(0,0);
  display.println("Target Game!");
  display.setTextSize(2);
  display.setCursor(0,20);
  display.print("Target:");
  display.setCursor(80,20);
  display.print(target);
  display.setTextSize(1);
  display.setCursor(0,50);
  display.println("Clap exactly the number!");
  display.display();
}

void updateDisplay() {
  display.fillRect(0,40,128,20,SSD1306_BLACK);
  display.setTextSize(2);
  display.setCursor(0,40);
  display.print("You:");
  display.setCursor(60,40);
  display.print(clapCount);
  display.display();
}

void showResult() {
  display.clearDisplay();
  display.setTextSize(2);
  display.setCursor(0,10);
  if (clapCount == target) {
    display.println("You Win!");
  } else if (clapCount < target) {
    display.println("Too few!");
  } else {
    display.println("Too many!");
  }
  display.setTextSize(1);
  display.setCursor(0,50);
  display.print("Target:");
  display.print(target);
  display.print(" You:");
  display.print(clapCount);
  display.display();
}
