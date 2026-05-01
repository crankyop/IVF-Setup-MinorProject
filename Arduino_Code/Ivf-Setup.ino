#include <ESP8266WiFi.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>

// WiFi credentials
const char* ssid = "Daksh";
const char* password = "12345678";

// ThingSpeak API Key
String apiKey = "YOUR_THINGSPEAK_API_KEY";

const char* server = "api.thingspeak.com";

// LCD
LiquidCrystal_I2C lcd(0x27, 16, 2);

// Pins
int sensorPin = D5;
int buzzerPin = D6;

// Variables
int dropCount = 0;
int totalDrops = 0;
unsigned long lastTime = 0;

bool lastState = HIGH;

WiFiClient client;

void setup() {
  pinMode(sensorPin, INPUT);
  pinMode(buzzerPin, OUTPUT);

  lcd.init();
  lcd.backlight();

  lcd.print("Connecting...");
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
  }

  lcd.clear();
  lcd.print("WiFi OK");
  delay(1500);
  lcd.clear();
}

void loop() {

  // Edge detection (accurate counting)
  bool currentState = digitalRead(sensorPin);

  if (lastState == HIGH && currentState == LOW) {
    dropCount++;
    totalDrops++;
  }

  lastState = currentState;

  // Every 15 sec
  if (millis() - lastTime > 15000) {

    int dripRate = dropCount * 4;

    // Status calculation
    String status;
    if (dripRate == 0) status = "NoFlow";
    else if (dripRate < 10) status = "Low";
    else status = "Normal";

    // WiFi Signal Strength
    int rssi = WiFi.RSSI();   // e.g. -40 strong, -80 weak

    // LCD display (clean)
    lcd.setCursor(0, 0);
    lcd.print("R:");
    lcd.print(dripRate);
    lcd.print(" T:");
    lcd.print(totalDrops);

    lcd.setCursor(0, 1);
    lcd.print(status);
    lcd.print(" WiFi:");
    lcd.print(rssi);

    lcd.print("   "); // clear leftovers

    // Buzzer logic
    if (dripRate == 0) {
      digitalWrite(buzzerPin, HIGH);  // continuous alert
    }
    else if (dripRate < 10) {
      digitalWrite(buzzerPin, HIGH);
      delay(150);
      digitalWrite(buzzerPin, LOW);
    }
    else {
      digitalWrite(buzzerPin, LOW);
    }

    // Send to ThingSpeak (only 2 fields)
    if (client.connect(server, 80)) {
      String url = "/update?api_key=" + apiKey +
                   "&field1=" + String(dripRate) +
                   "&field2=" + String(totalDrops);

      client.print(String("GET ") + url + " HTTP/1.1\r\n" +
                   "Host: " + server + "\r\n" +
                   "Connection: close\r\n\r\n");
    }

    dropCount = 0;
    lastTime = millis();
  }
}
