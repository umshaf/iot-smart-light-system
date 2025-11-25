const int SENSOR_PIN = A0;
const int LED_PIN = 6;

int threshold = 260; 

void setup() {
  Serial.begin(115200);
  pinMode(LED_PIN, OUTPUT);
  Serial.println("LDR Test Starting...");
}

void loop() {
  int value = analogRead(SENSOR_PIN);
  Serial.print("LDR Value: ");
  Serial.println(value);

  if (value < threshold) {
    digitalWrite(LED_PIN, HIGH);  // dark -> LED on
  } else {
    digitalWrite(LED_PIN, LOW);   // bright -> LED off
  }

  delay(200);
}
