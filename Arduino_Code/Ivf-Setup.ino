#include <ESP8266WiFi.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>

// WiFi
const char* ssid = "Daksh";
const char* password = "12345678";

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

// ✨ Scroll text function
void scrollText(String text, int row) {
  for (int i = 0; i < text.length() - 15; i++) {
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

  // ✨ Startup animation
  scrollText(" Smart IV Monitoring System ", 0);

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

  // Drop detection
  bool currentState = digitalRead(sensorPin);

  if (lastState == HIGH && currentState == LOW) {
    dropCount++;
    totalDrops++;
  }

  lastState = currentState;

  // Every 15 sec
  if (millis() - lastTime > 15000) {

    int dripRate = dropCount * 4;

    // Status
    String status;
    if (dripRate == 0) status = "NoFlow";
    else if (dripRate < 10) status = "Low";
    else status = "Normal";

    int rssi = WiFi.RSSI();

    // ✨ Smooth LCD update
    lcd.setCursor(0, 0);
    lcd.print("R:");
    lcd.print(dripRate);
    lcd.print(" T:");
    lcd.print(totalDrops);
    lcd.print("   ");

    lcd.setCursor(0, 1);
    lcd.print(status);
    lcd.print(" WiFi:");
    lcd.print(rssi);
    lcd.print("   ");

    // Buzzer
    if (dripRate == 0) {
      digitalWrite(buzzerPin, HIGH);
    }
    else if (dripRate < 10) {
      digitalWrite(buzzerPin, HIGH);
      delay(150);
      digitalWrite(buzzerPin, LOW);
    }
    else {
      digitalWrite(buzzerPin, LOW);
    }

    // Send data
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
