#include <WiFiS3.h>

const char* ssid     = "YOUR_WIFI_NAME";
const char* password = "YOUR_WIFI_PASSWORD";

const char* server    = "api.thingspeak.com";
String apiKey         = "YOUR_API_KEY";  
long channelID        = 3181117;             

const int SENSOR_PIN = A0;   

unsigned long lastUpdate = 0;
const unsigned long updateInterval = 20000; 

void setup() {
  Serial.begin(115200);
  delay(1000);

  Serial.println("Smart Light -> ThingSpeak");
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

void loop() {
  int lightValue = analogRead(SENSOR_PIN);
  Serial.print("LDR Value: ");
  Serial.println(lightValue);

  unsigned long now = millis();
  if (now - lastUpdate >= updateInterval && WiFi.status() == WL_CONNECTED) {
    sendToThingSpeak(lightValue);
    lastUpdate = now;
  }

  delay(500);
}

void sendToThingSpeak(int value) {
  WiFiClient client;

  Serial.println("Connecting to ThingSpeak...");

  if (!client.connect(server, 80)) {
    Serial.println("Connection to ThingSpeak failed.");
    return;
  }

  String url = "/update?api_key=" + apiKey + "&field1=" + String(value);

  Serial.print("Requesting URL: ");
  Serial.println(url);

  client.print(String("GET ") + url + " HTTP/1.1\r\n");
  client.print("Host: api.thingspeak.com\r\n");
  client.print("Connection: close\r\n\r\n");

  while (client.connected()) {
    while (client.available()) {
      char c = client.read();
    }
  }

  client.stop();
  Serial.println("Update sent.\n");
}
