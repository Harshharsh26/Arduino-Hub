#include <Servo.h>

Servo myServo;

void setup() {
  myServo.attach(9);       // Attach servo to pin 9
  myServo.write(0);        // Start at 0 degrees
}

void loop() {
  myServo.write(90);       // Move to 90 degrees
  delay(1000);
  myServo.write(180);      // Move to 180 degrees
  delay(1000);
  myServo.write(0);        // Move back to 0 degrees
  delay(1000);
}
