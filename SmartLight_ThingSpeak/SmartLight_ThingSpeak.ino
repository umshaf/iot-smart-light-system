#include <WiFiS3.h>

// ------------------------
// WiFi + ThingSpeak setup
// ------------------------
const char* ssid     = "YOUR-WIFI";       
const char* password = "YOUR-WIFI-PASSWORD";      

const char* server   = "api.thingspeak.com";
String apiKey        = "YOUR-API-KEY";    

// ------------------------
// Hardware pins
// ------------------------
const int SENSOR_PIN = A0;   // LDR
const int LED_PIN    = 6;    // LED on D6

// ------------------------
// Edge AI: rolling average
// ------------------------
const int WINDOW_SIZE    = 10;   // number of samples in moving window
const int OFFSET         = 200;  // how far below average the threshold sits
const int MIN_THRESHOLD  = 300;  // don't let threshold go below this

int   readings[WINDOW_SIZE];
int   readIndex   = 0;
long  total       = 0;
int   numReadings = 0;

int   avgValue    = 0;
int   threshold   = 0;

// ------------------------
// ThingSpeak timing
// ------------------------
unsigned long lastUpdate      = 0;
const unsigned long updateInterval = 20000; // 20 seconds

// ------------------------
// Setup
// ------------------------
void setup() {
  Serial.begin(115200);
  delay(1000);

  pinMode(LED_PIN, OUTPUT);

  // init rolling average buffer
  for (int i = 0; i < WINDOW_SIZE; i++) {
    readings[i] = 0;
  }

  Serial.println("Smart Light -> ThingSpeak (Edge AI)");

  // Connect to WiFi
  Serial.print("Connecting to WiFi: ");
  Serial.println(ssid);

  WiFi.begin(ssid, password);

  int maxTries = 30;
  while (WiFi.status() != WL_CONNECTED && maxTries > 0) {
    delay(500);
    Serial.print(".");
    maxTries--;
  }

  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("\nWiFi connected.");
    Serial.print("IP address: ");
    Serial.println(WiFi.localIP());
  } else {
    Serial.println("\nWiFi connection FAILED. Check SSID/password.");
  }
}

// ------------------------
// Main loop
// ------------------------
void loop() {
  int lightValue = analogRead(SENSOR_PIN);

  // --- rolling average update ---
  total -= readings[readIndex];        // remove oldest
  readings[readIndex] = lightValue;    // store new
  total += readings[readIndex];        // add newest

  readIndex++;
  if (readIndex >= WINDOW_SIZE) {
    readIndex = 0;
  }

  if (numReadings < WINDOW_SIZE) {
    numReadings++;
  }

  avgValue = total / numReadings;
  threshold = avgValue - OFFSET;

  // clamp threshold so it never gets too low
  if (threshold < MIN_THRESHOLD) {
    threshold = MIN_THRESHOLD;
  }

  // --- LED control ---
  if (lightValue < threshold) {
    digitalWrite(LED_PIN, HIGH);  // dark -> LED ON
  } else {
    digitalWrite(LED_PIN, LOW);   // bright -> LED OFF
  }

  // --- Serial debug output ---
  Serial.print("LDR: ");
  Serial.print(lightValue);
  Serial.print("  Avg: ");
  Serial.print(avgValue);
  Serial.print("  Thr: ");
  Serial.print(threshold);
  Serial.print("  LED: ");
  Serial.println(lightValue < threshold ? "ON" : "OFF");

  // --- ThingSpeak upload every 20 s ---
  unsigned long now = millis();
  if (now - lastUpdate >= updateInterval && WiFi.status() == WL_CONNECTED) {
    sendToThingSpeak(lightValue);
    lastUpdate = now;
  }

  delay(500);
}

// ------------------------
// ThingSpeak upload
// ------------------------
void sendToThingSpeak(int value) {
  WiFiClient client;

  Serial.println("Connecting to ThingSpeak...");

  if (!client.connect(server, 80)) {
    Serial.println("Connection to ThingSpeak failed.");
    return;
  }

  // Build URL with 3 fields: LDR, Avg, Threshold
  String url = "/update?api_key=" + apiKey +
               "&field1=" + String(value) +
               "&field2=" + String(avgValue) +
               "&field3=" + String(threshold);

  Serial.print("Sending request: GET ");
  Serial.println(url);

  client.print("GET " + url + " HTTP/1.1\r\n");
  client.print("Host: api.thingspeak.com\r\n");
  client.print("Connection: close\r\n\r\n");

  // --- read and print ThingSpeak response ---
  unsigned long timeout = millis();
  String response = "";

  while (client.connected() && millis() - timeout < 5000) {
    while (client.available()) {
      char c = client.read();
      response += c;
      timeout = millis(); // reset timeout when data received
    }
  }

  client.stop();

  Serial.println("Response from ThingSpeak:");
  Serial.println(response);
  Serial.println();
}

