#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <DHT.h>

// OLED
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

// DHT11
#define DHTPIN 4
#define DHTTYPE DHT11
DHT dht(DHTPIN, DHTTYPE);

// LEDs
#define LED_VERDE 15
#define LED_AMARILLO 5
#define LED_ROJO 18

void setup() {
  Serial.begin(115200);

  dht.begin();

  pinMode(LED_VERDE, OUTPUT);
  pinMode(LED_AMARILLO, OUTPUT);
  pinMode(LED_ROJO, OUTPUT);

  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println("OLED no detectado");
    while (true);
  }

  display.clearDisplay();
}

void loop() {

  float temp = dht.readTemperature();
  float hum = dht.readHumidity();

  if (isnan(temp) || isnan(hum)) {
    Serial.println("Error leyendo sensor");
    return;
  }

  String estado = "";

  // Apagar todos los LEDs
  digitalWrite(LED_VERDE, LOW);
  digitalWrite(LED_AMARILLO, LOW);
  digitalWrite(LED_ROJO, LOW);

  // Lógica de temperatura
  if (temp < 15) {
    estado = "FRIA";
    digitalWrite(LED_AMARILLO, HIGH);
  } 
  else if (temp >= 15 && temp <= 25) {
    estado = "BUENA";
    digitalWrite(LED_VERDE, HIGH);
  } 
  else {
    estado = "ALTA";
    digitalWrite(LED_ROJO, HIGH);
  }

  // OLED
  display.clearDisplay();

  display.setTextSize(1);
  display.setTextColor(WHITE);

  display.setCursor(0, 5);
  display.print("Temp: ");
  display.print(temp);
  display.println(" C");

  display.setCursor(0, 20);
  display.print("Hum: ");
  display.print(hum);
  display.println(" %");

  display.setCursor(0, 40);
  display.print("Estado: ");
  display.println(estado);

  display.display();

  delay(2000);
}