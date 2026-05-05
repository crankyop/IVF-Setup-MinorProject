#include <ESP8266WiFi.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>

// WiFi
const char* ssid = "Daksh";
const char* password = "12345678";

// ThingSpeak API Key
String apiKey = "Key_Here";
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

// Startup scroll
void scrollText(String text, int row) {
  for (int i = 0; i <= text.length() - 16; i++) {
    lcd.setCursor(0, row);
    lcd.print(text.substring(i, i + 16));
    delay(250);
  }
}

void setup() {
  pinMode(sensorPin, INPUT);
  pinMode(buzzerPin, OUTPUT);

  lcd.init();
  lcd.backlight();

  scrollText(" Smart IVF Monitoring System ", 0);

  lcd.clear();
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
  bool currentState = digitalRead(sensorPin);

  if (lastState == HIGH && currentState == LOW) {
    dropCount++;
    totalDrops++;
  }

  lastState = currentState;

  // Update every 15 sec
  if (millis() - lastTime > 15000) {

    int dripRate = dropCount * 4;

    // LCD display
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Drop Rate:");
    lcd.print(dripRate);

    lcd.setCursor(0, 1);
    lcd.print("Total:");
    lcd.print(totalDrops);

    // Buzzer
    if (dripRate == 0) {
      digitalWrite(buzzerPin, HIGH);
    } else {
      digitalWrite(buzzerPin, LOW);
    }

    // Send to ThingSpeak
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
