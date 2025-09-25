int micAnalog = A0;
int soundValue;

void setup() {
  Serial.begin(9600);
}

void loop() {
  soundValue = analogRead(micAnalog);
  Serial.println(soundValue);
  delay(200);
}
