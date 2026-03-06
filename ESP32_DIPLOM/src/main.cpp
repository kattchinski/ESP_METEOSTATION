#include <Arduino.h>
#include <WiFi.h>
#include <SPIFFS.h>
#include <Firebase_ESP_Client.h>
#include <Adafruit_BME280.h>
#include <LiquidCrystal_I2C.h>
#include <WebServer.h>
#include "addons/TokenHelper.h"
#include "web.h"

// --- НАЛАШТУВАННЯ ---
#define WIFI_SSID     "V3"
#define WIFI_PASS     "SjG3pm7u89RX"
#define API_KEY       "AIzaSyC9ti9KEQXXdUs6rhc1io68BPIxwYBR3Sc"
#define DATABASE_URL  "https://meteoespenitration-default-rtdb.europe-west1.firebasedatabase.app/"
#define USER_EMAIL    "esp32@gmail.com"
#define USER_PASSWORD "123456"

#define I2C_SDA 21
#define I2C_SCL 22
const int touchBtn = 25; 

WebServer server(80);
Adafruit_BME280 bme;
LiquidCrystal_I2C* lcd = nullptr;
FirebaseData fbdo;
FirebaseConfig cfg;
FirebaseAuth auth;

String devicePath;
unsigned long lastFirebaseSend = 0;
unsigned long lastScreenUpdate = 0;
unsigned long lastTimeUpdate = 0;
int screenIndex = 1;
const int screenCount = 4;
bool lastTouchState = false;

float pressureStat[9];
float resultStat = 0;

// --- ФУНКЦІЇ ДЕБАГУ ТА ЛОГІКИ ---

void handleData() {
  Serial.println("[WEB] Request received: /data.json");
  float t = bme.readTemperature();
  float p = bme.readPressure() / 100.0F;

  String json = "{\"temp\":";
  json += String(t, 1);
  json += ",\"press\":";
  json += String(p, 1);
  json += "}";

  server.send(200, "application/json", json);
  Serial.printf("[WEB] Sent: T=%.1f, P=%.1f\n", t, p);
}

void showScreen(int idx) {
  if (!lcd) return;
  //Serial.printf("[LCD] Switching to screen: %d\n", idx);
  lcd->clear();
  switch (idx) {
    case 1: {
      unsigned long sec = millis() / 1000;
      lcd->setCursor(0, 0); lcd->print("ESP32 Meteo");
      lcd->setCursor(0, 1); lcd->printf("Upt: %02u:%02u:%02u", (unsigned int)(sec/3600), (unsigned int)((sec%3600)/60), (unsigned int)(sec%60));
      break;
    }
    case 2: {
      lcd->setCursor(0, 0); lcd->print("T: "); lcd->print(bme.readTemperature(), 1); lcd->print(" C");
      lcd->setCursor(0, 1); lcd->print("P: "); lcd->print(bme.readPressure()/100.0, 1); lcd->print(" hPa");
      break;
    }
    case 3: {
      int p = (int)(resultStat / 100.0);
      lcd->setCursor(0, 0); lcd->print("Risk of Rain:");
      lcd->setCursor(0, 1);
      if (p > 1020) lcd->print("FAIR & DRY");
      else if (p > 1010) lcd->print("NORMAL");
      else if (p > 1000) lcd->print("WET&CLOUDY");
      else lcd->print("GET WET SOON");
      break;
    }
    case 4: {
      lcd->setCursor(0, 0); lcd->print("Firebase:");
      lcd->setCursor(0, 1); lcd->print(Firebase.ready() ? "Online" : "Connecting...");
      break;
    }
  }
}

// --- ФУНКЦІЯ ЧИТАННЯ ID ПРИСТРОЮ ---
String chipIdStr() {
  uint64_t id = ESP.getEfuseMac();
  char buf[13];
  // Перетворюємо MAC-адресу в гарний рядок великими літерами
  snprintf(buf, sizeof(buf), "%012llX", (unsigned long long)id);
  return String(buf);
}

void setup() {
  Serial.begin(115200);
  delay(2000); 
  Serial.println("\n\n=== BOOTING ESP32 METEO STATION ===");
  
  Wire.begin(I2C_SDA, I2C_SCL);
  pinMode(touchBtn, INPUT);

  // LCD Scan
  Serial.println("[I2C] Scanning for LCD...");
  uint8_t addr = 0x27; // спроба за дефолтом
  Wire.beginTransmission(0x27);
  if (Wire.endTransmission() != 0) {
    Wire.beginTransmission(0x3F);
    if (Wire.endTransmission() == 0) addr = 0x3F;
    else addr = 0;
  }

  if (addr) {
    Serial.printf("[I2C] LCD found at 0x%02X\n", addr);
    lcd = new LiquidCrystal_I2C(addr, 16, 2);
    lcd->init();
    lcd->backlight();
    lcd->print("System Booting...");
  } else {
    Serial.println("[ERROR] LCD not found!");
  }

  // BME280
  Serial.println("[I2C] Testing BME280...");
  if (!bme.begin(0x76) && !bme.begin(0x77)) {
    Serial.println("[CRITICAL] BME280 sensor missing!");
    if (lcd) lcd->print("BME280 ERROR");
    while(1) delay(1000);
  }
  Serial.println("[I2C] BME280 OK");

  // WiFi
  Serial.printf("[WiFi] Connecting to %s ", WIFI_SSID);
  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASS);
  int attempts = 0;
  while (WiFi.status() != WL_CONNECTED && attempts < 20) {
    delay(500); Serial.print("."); attempts++;
  }
  
  if(WiFi.status() == WL_CONNECTED) {
    Serial.printf("\n[WiFi] Connected! IP: %s\n", WiFi.localIP().toString().c_str());
  } else {
    Serial.println("\n[ERROR] WiFi Connection failed!");
  }

  // SPIFFS (для токенів)
  if (!SPIFFS.begin(true)) Serial.println("[FS] SPIFFS Mount Failed");
  else Serial.println("[FS] SPIFFS OK");

  // Firebase Init
  Serial.println("[Firebase] Initializing...");
  cfg.api_key = API_KEY;
  cfg.database_url = DATABASE_URL;
  auth.user.email = USER_EMAIL;
  auth.user.password = USER_PASSWORD;
  cfg.token_status_callback = tokenStatusCallback;
  
  Firebase.begin(&cfg, &auth);
  Firebase.reconnectWiFi(true);
  fbdo.setResponseSize(4096);

  devicePath = String("/devices/");
  devicePath += String(chipIdStr());
  devicePath += String("/telemetry");
  
  Serial.printf("[Firebase] Device Path: %s\n", devicePath.c_str());

  // Web Server
  server.on("/", []() { 
    Serial.println("[WEB] Index page requested");
    server.send_P(200, "text/html", MAIN_page); 
  });
  server.on("/data.json", handleData);
  server.begin();
  Serial.println("[WEB] Server started on port 80");

  for(int i=0; i<9; i++) pressureStat[i] = bme.readPressure();
  showScreen(screenIndex);
  Serial.println("=== SETUP COMPLETE ===\n");
}

void loop() {
  server.handleClient();
  
  // Статистика тиску (кожні 5 сек)
  static unsigned long lastStatUpdate = 0;
  if (millis() - lastStatUpdate > 5000) {
    pressureStat[0] = bme.readPressure(); // спрощено для дебагу
    float sum = 0;
    for (int i = 0; i < 9; i++) sum += pressureStat[i];
    resultStat = sum / 9.0f;
    lastStatUpdate = millis();
  }

  // 1. Firebase Push
  if (millis() - lastFirebaseSend >= 15000) {
    if (Firebase.ready()) {
      Serial.println("[Firebase] Pushing telemetry...");
      FirebaseJson j;
      j.set("temp", bme.readTemperature());
      j.set("press", bme.readPressure() / 100.0);
      
      if (Firebase.RTDB.pushJSON(&fbdo, devicePath.c_str(), &j)) {
        Serial.printf("[Firebase] Push OK. Path: %s\n", fbdo.dataPath().c_str());
      } else {
        Serial.printf("[ERROR] Firebase Push Failed: %s\n", fbdo.errorReason().c_str());
      }
    } else {
      Serial.println("[Firebase] Not ready (token issue or no internet)");
    }
    lastFirebaseSend = millis();
  }

  // 2. Button Check
  bool touchState = digitalRead(touchBtn);
  if (touchState == HIGH && !lastTouchState) {
    Serial.println("[INPUT] Touch detected!");
    screenIndex = (screenIndex % screenCount) + 1;
    showScreen(screenIndex);
    lastScreenUpdate = millis();
  }
  lastTouchState = touchState;

  // 3. Screen Auto-update
  if (screenIndex == 1 && millis() - lastTimeUpdate >= 1000) {
    showScreen(1);
    lastTimeUpdate = millis();
  } else if (millis() - lastScreenUpdate >= 10000) {
    showScreen(screenIndex);
    lastScreenUpdate = millis();
  }

  delay(10);
}