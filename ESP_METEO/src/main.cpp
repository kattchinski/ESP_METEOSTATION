#include <Arduino.h>
#include <WiFi.h>
#include <SPIFFS.h>
#include <Firebase_ESP_Client.h>
#include <Adafruit_BME280.h>
#include "addons/TokenHelper.h"   // тільки для логів

#define WIFI_SSID     "V3"
#define WIFI_PASS     "SjG3pm7u89RX"
#define API_KEY       "AIzaSyC9ti9KEQXXdUs6rhc1io68BPIxwYBR3Sc"
#define DATABASE_URL  "https://meteoespenitration-default-rtdb.europe-west1.firebasedatabase.app/"
#define USER_EMAIL    "esp32@gmail.com"
#define USER_PASSWORD "123456"

FirebaseData fbdo;
FirebaseConfig cfg;
FirebaseAuth auth;

unsigned long lastSend = 0;

// --------- Піни / налаштування ---------
#define I2C_SDA 21
#define I2C_SCL 22
Adafruit_BME280 bme; 
static bool waitUntil(std::function<bool()> ok, uint32_t ms, const char* what) {
  uint32_t t0 = millis();
  Serial.printf("Waiting for %s ", what);
  while (!ok()) {
    if (millis() - t0 > ms) { Serial.println("-> TIMEOUT"); return false; }
    Serial.print(".");
    delay(200);
  }
  Serial.println("-> OK");
  return true;
}

// Формує стабільний читабельний ID пристрою на основі заводської MAC-адреси ESP32.
// Результат: 12 шістнадцяткових символів у ВЕРХНЬОМУ регістрі, напр. "A4CF12ABCDEF".
String chipIdStr(){
  // Зчитуємо 48-бітну MAC-адресу з eFuse (повертається як 64-бітне число)
  uint64_t id=ESP.getEfuseMac();
  // Буфер під 12 шістнадцяткових символів + нуль-термінатор = 13 байт.
  char buf[13];
  // Форматуємо значення як шістнадцятковий рядок фіксованої довжини 12,
  // з доповненням нулями зліва та великими літерами.
  // %012llX: ціле без знаку, ширина 12, шістнадцятковий, верхній регістр.
  snprintf(buf,sizeof(buf),"%012llX",(unsigned long long)id);
  // Обгортаємо C-рядок у Arduino String і повертаємо.
  return String(buf);
}
String devicePath;

void setup() {
  Serial.begin(115200);
  // Ініцалізація I2C
  Wire.begin(I2C_SDA, I2C_SCL);
  delay(300);
    // ---- BME280 ----
  if (!bme.begin(0x76)) {
    if (!bme.begin(0x77)) {
      Serial.println("No BME280 at 0x76/0x77.");
      while (1) delay(100);
    }
  }
  Serial.println("\nBOOT");

  // FS для кешу токена (безпечний старт)
  if (!SPIFFS.begin(true)) Serial.println("SPIFFS FAIL"); else Serial.println("SPIFFS OK");

  // Wi-Fi
  WiFi.mode(WIFI_STA);
  WiFi.setAutoReconnect(true);
  WiFi.begin(WIFI_SSID, WIFI_PASS);
  if (!waitUntil([]{ return WiFi.status()==WL_CONNECTED; }, 15000, "WiFi")) return;
  Serial.println(WiFi.localIP().toString());

  // Firebase
  cfg.api_key = API_KEY;
  cfg.database_url = DATABASE_URL;
  cfg.token_status_callback = tokenStatusCallback; // побачиш стадії токена
  auth.user.email = USER_EMAIL;
  auth.user.password = USER_PASSWORD;

  fbdo.setResponseSize(2048);

  Serial.println("Firebase.begin()");
  Firebase.begin(&cfg, &auth);
  Firebase.reconnectWiFi(true);

  // чекаємо готовність токена (інакше set/get часто в нікуди)
  if (!waitUntil([]{ return Firebase.ready(); }, 20000, "Firebase token")) return;

  // Пінг: дешевий запис у /ping
  Serial.println("Write /ping ...");
  bool ok = Firebase.RTDB.setString(&fbdo, "/ping", "ok");
  if (ok) Serial.println("FIREBASE OK");
  else    Serial.printf("FIREBASE FAIL: %s\n", fbdo.errorReason().c_str());

  // Build devicePath without using chained + operators to avoid
  // ambiguous operator+ overloads from different String implementations.
  {
    String dp = String("/devices/");
    dp.concat(chipIdStr());
    dp.concat("/telemetry");
    devicePath = dp;
  }
}

void loop() 
{
  if (millis()-lastSend >= 5000 && Firebase.ready()) {
    float tempr = bme.readTemperature();      // °C
    float airPressure = bme.readPressure();   // Па
    lastSend = millis();

    FirebaseJson j;
    j.set("temp", tempr);
    j.set("press", airPressure);
    bool ok = Firebase.RTDB.pushJSON(&fbdo, devicePath.c_str(), &j);
    Serial.println(ok ? "push OK" : fbdo.errorReason());
  }
}
