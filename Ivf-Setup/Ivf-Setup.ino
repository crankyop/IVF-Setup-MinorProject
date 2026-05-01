#include <ESP8266WiFi.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>

// WiFi credentials
const char* ssid = "Daksh";
const char* password = "12345678";

// ThingSpeak API Key
String apiKey = "ZLDL4CZO6KAI73NI";

const char* server = "api.thingspeak.com";

// LCD setup
LiquidCrystal_I2C lcd(0x27, 16, 2);

// Pins
int sensorPin = D5;
int buzzerPin = D6;

// Variables
int dropCount = 0;
int totalDrops = 0;
unsigned long lastTime = 0;

WiFiClient client;

void setup() {
  pinMode(sensorPin, INPUT);
  pinMode(buzzerPin, OUTPUT);

  lcd.init();
  lcd.backlight();
  lcd.print("Connecting WiFi");

  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
  }

  lcd.clear();
  lcd.print("WiFi Connected");
  delay(1500);
  lcd.clear();
}

void loop() {

  // Detect drops
  if (digitalRead(sensorPin) == LOW) {
    dropCount++;
    totalDrops++;
    delay(200);
  }

  // Every 15 seconds
  if (millis() - lastTime > 15000) {

    int dripRate = dropCount * 4;

    // Status & Alert
    String status;
    String alert;

    if (dripRate == 0) {
      status = "No Flow";
      alert = "ALERT";
      digitalWrite(buzzerPin, HIGH);
    }
    else if (dripRate < 10) {
      status = "Low";
      alert = "ALERT";
      digitalWrite(buzzerPin, LOW);
    }
    else {
      status = "Normal";
      alert = "OK";
      digitalWrite(buzzerPin, LOW);
    }

    // LCD display
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Rate:");
    lcd.print(dripRate);

    lcd.setCursor(0, 1);
    lcd.print("Tot:");
    lcd.print(totalDrops);

    // Send to ThingSpeak
    if (client.connect(server, 80)) {
      String url = "/update?api_key=" + apiKey +
                   "&field1=" + String(dripRate) +
                   "&field2=" + String(totalDrops) +
                   "&field3=" + status +
                   "&field4=" + alert;

      client.print(String("GET ") + url + " HTTP/1.1\r\n" +
                   "Host: " + server + "\r\n" +
                   "Connection: close\r\n\r\n");
    }

    dropCount = 0;
    lastTime = millis();
  }
}