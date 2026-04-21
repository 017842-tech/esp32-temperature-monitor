#include <WiFi.h>
#include <HTTPClient.h>
#include <DHT.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include "time.h"

// 🔌 WiFi
const char* ssid = "IZZI-DA32";
const char* password = "50A5DC29DA32";

// 🌐 Firebase
const char* serverName = "https://firestore.googleapis.com/v1/projects/analysis-8506e/databases/(default)/documents/temperature";

// 🌡️ DHT11
#define DHTPIN 4
#define DHTTYPE DHT11
DHT dht(DHTPIN, DHTTYPE);

// 📺 OLED
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

// 💡 LEDs
int greenLED = 18;
int yellowLED = 5;
int redLED = 2;

// 🌍 NTP (Mexico UTC -6)
const char* ntpServer = "pool.ntp.org";
const long gmtOffset_sec = -21600;
const int daylightOffset_sec = 0;

void setup() {
  Serial.begin(115200);
  delay(1000);

  Serial.println("INICIANDO...");

  dht.begin();

  pinMode(greenLED, OUTPUT);
  pinMode(yellowLED, OUTPUT);
  pinMode(redLED, OUTPUT);

  // 📡 WiFi
  Serial.print("Conectando WiFi...");
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("\nWiFi conectado!");
  Serial.print("IP: ");
  Serial.println(WiFi.localIP());

  // 🕒 TIME (FIXED)
  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);

  Serial.print("Sincronizando hora");
  time_t now = time(nullptr);

int retries = 0;
while (now < 100000 && retries < 10) {
  delay(500);
  Serial.print(".");
  now = time(nullptr);
  retries++;
}

if (now < 100000) {
  Serial.println("\n⚠️ Failed to sync time, continuing anyway...");
} else {
  Serial.println("\nHora sincronizada!");
}

  Serial.println("\nHora sincronizada!");

  // 📺 OLED
  Wire.begin(21, 22);

  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println("ERROR OLED");
    while (true);
  }

  display.clearDisplay();
}

void loop() {

  float temp = dht.readTemperature();
  float hum = dht.readHumidity();

  if (isnan(temp) || isnan(hum)) {
    Serial.println("Error leyendo DHT11");
    delay(2000);
    return;
  }

  String estado = "";

  // 🔥 LÓGICA LEDS
  if (temp > 25) {
    estado = "ALTA";
    digitalWrite(redLED, HIGH);
    digitalWrite(yellowLED, LOW);
    digitalWrite(greenLED, LOW);

  } else if (temp >= 15) {
    estado = "BUENA";
    digitalWrite(redLED, LOW);
    digitalWrite(yellowLED, LOW);
    digitalWrite(greenLED, HIGH);

  } else {
    estado = "BAJA";
    digitalWrite(redLED, LOW);
    digitalWrite(yellowLED, HIGH);
    digitalWrite(greenLED, LOW);
  }

  // 📺 OLED DISPLAY
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(WHITE);

  display.setCursor(0, 10);
  display.print("Temp: ");
  display.print(temp);
  display.println(" C");

  display.setCursor(0, 25);
  display.print("Hum: ");
  display.print(hum);
  display.println(" %");

  display.setCursor(0, 45);
  display.print("Estado: ");
  display.println(estado);

  display.display();

  // 🕒 REAL TIMESTAMP
  time_t now = time(nullptr);
  long timestamp = now;

  // 🌐 SEND TO FIREBASE
  if (WiFi.status() == WL_CONNECTED) {
    HTTPClient http;

    http.begin(serverName);
    http.addHeader("Content-Type", "application/json");

    String jsonData = "{ \"fields\": { ";
    jsonData += "\"value\": { \"doubleValue\": " + String(temp) + " }, ";
    jsonData += "\"humidity\": { \"doubleValue\": " + String(hum) + " }, ";
    jsonData += "\"status\": { \"stringValue\": \"" + estado + "\" }, ";
    jsonData += "\"timestamp\": { \"integerValue\": \"" + String(timestamp) + "\" } ";
    jsonData += "} }";

    int httpResponseCode = http.POST(jsonData);

    Serial.print("Firebase response: ");
    Serial.println(httpResponseCode);

    http.end();
  }

  // 🖥️ DEBUG
  Serial.print("Temp: ");
  Serial.print(temp);
  Serial.print(" °C | Hum: ");
  Serial.print(hum);
  Serial.print("% | Estado: ");
  Serial.println(estado);

  delay(5000);
}