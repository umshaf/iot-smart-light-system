const int SENSOR_PIN = A0;
const int LED_PIN = 6;

const int NUM_READINGS = 20;
int readings[NUM_READINGS];
int indexPos = 0;
bool bufferFilled = false;

void setup() {
  Serial.begin(115200);
  pinMode(LED_PIN, OUTPUT);

  int initial = analogRead(SENSOR_PIN);
  for (int i = 0; i < NUM_READINGS; i++) {
    readings[i] = initial;
  }

  Serial.println("Edge AI Smart Light starting...");
}

void loop() {
  int current = analogRead(SENSOR_PIN);

  readings[indexPos] = current;
  indexPos++;

  if (indexPos >= NUM_READINGS) {
    indexPos = 0;
    bufferFilled = true;
  }

  long sum = 0;
  int count = bufferFilled ? NUM_READINGS : indexPos;

  if (count == 0) count = 1;

  for (int i = 0; i < count; i++) {
    sum += readings[i];
  }

  int avg = sum / count;

  int offset = 50;                  
  int adaptiveThreshold = avg - offset;

  if (current < adaptiveThreshold) {
    digitalWrite(LED_PIN, HIGH);    
  } else {
    digitalWrite(LED_PIN, LOW);     
  }

  Serial.print("Current: ");
  Serial.print(current);
  Serial.print(" | Avg: ");
  Serial.print(avg);
  Serial.print(" | Thr: ");
  Serial.println(adaptiveThreshold);

  delay(200);
}
