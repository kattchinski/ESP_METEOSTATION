#include <Arduino.h>
#include <Wire.h>
#include <WiFi.h>
#include <WebServer.h>
#include <LiquidCrystal_I2C.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BME280.h>
#include "web.h"


// ======= WiFi =======
const char* ssid = "selfish";
const char* password = "0631111618";
WebServer server(80);


// --------- Піни / налаштування ---------
#define I2C_SDA 21
#define I2C_SCL 22
const int touchBtn = 25;  // сенсорна кнопка (замість GPIO3). Якщо в тебе інакше — зміни тут.

Adafruit_BME280 bme;

// LCD робимо вказівником, бо адресу визначимо під час роботи (0x27 або 0x3F)
LiquidCrystal_I2C* lcd = nullptr;

// Назви днів (залишаю для сумісності з твоєю структурою, але RTC не використовується)
//const char* WeekDays[] = {"Mon","Tue","Wed","Thu","Fri","Sat","Sun"};


// ======= JSON handler for Web =======
void handleData() {
  float t = bme.readTemperature();
  float p = bme.readPressure() / 100.0F;
  String json = "{\"temp\":" + String(t, 1) + ",\"press\":" + String(p, 1) + "}";
  server.send(200, "application/json", json);
}


// Логіка екранів та перемикання
unsigned long lastScreenUpdate = 0;
const unsigned long screenDelay = 10000; // 10 секунд
unsigned long lastTimeUpdate = 0;
bool dateDrawn = false; // для першого екрану (зараз Uptime)
int screenIndex = 1;
const unsigned int screenCount = 4;

// Статистика тиску (для «прогнозу»)
float pressureStat[9];
float resultStat = 0;

// --------- Службові ---------
bool lastTouchState = false;

// --------- Допоміжні функції ---------
uint8_t detectLcdAddr() {
  // Швидкий чек двох найтиповіших адрес
  const uint8_t candidates[2] = {0x27, 0x3F};
  for (uint8_t addr : candidates) {
    Wire.beginTransmission(addr);
    if (Wire.endTransmission() == 0) return addr;
  }
  return 0; // не знайдено
}

void fillPressureBufferOnce() {
  for (int i = 0; i < 9; i++) {
    pressureStat[i] = bme.readPressure(); // Па
    delay(5);
  }
  resultStat = bme.readPressure();
}

void Statistic() {
  static short index = 0;
  static unsigned long lastUpdate = 0;
  if (millis() - lastUpdate > 5000) {          // раз на 5 секунд
    pressureStat[index] = bme.readPressure();  // Па
    index = (index + 1) % 9;
    lastUpdate = millis();
  }

  // середнє
  float sum = 0;
  for (int i = 0; i < 9; i++) sum += pressureStat[i];
  resultStat = sum / 9.0f;
}

void ShowScreen(int idx) {
  if (!lcd) return;
  lcd->clear();

  switch (idx) {
    case 1: { // Замість RTC показуємо Uptime (год:хв:сек)
      unsigned long seconds = millis() / 1000;
      unsigned int h = seconds / 3600;
      unsigned int m = (seconds % 3600) / 60;
      unsigned int s = seconds % 60;

      lcd->setCursor(0, 0);
      lcd->print("ESP32 Meteo");
      lcd->setCursor(0, 1);
      lcd->print("Uptime ");
      if (h < 10) lcd->print('0'); lcd->print(h); lcd->print(':');
      if (m < 10) lcd->print('0'); lcd->print(m); lcd->print(':');
      if (s < 10) lcd->print('0'); lcd->print(s);
      break;
    }

    case 2: { // Температура + Тиск (як у тебе)
      float tempr = bme.readTemperature();      // °C
      float airPressure = bme.readPressure();   // Па

      lcd->setCursor(0, 0);
      lcd->print("Temperature:");
      lcd->setCursor(0, 1);
      lcd->print(tempr, 1);
      lcd->print((char)223); // знак °C
      lcd->print("C");

      // Перенесемо тиск у куток першого рядка, щоб влізло
      // або очистимо і виведемо другим екраном — лишаю, як було в тебе:
      delay(1200);
      lcd->clear();
      lcd->setCursor(0, 0);
      lcd->print("Press:");
      lcd->print(airPressure / 100.0, 1); // в hPa
      lcd->print("hPa");
      break;
    }

    case 3: { // «Прогноз» за середнім тиском
      int pressureHpa = (int)(resultStat / 100.0); // Па -> hPa

      lcd->setCursor(0, 0);
      lcd->print("Risk of Rain:");
      lcd->setCursor(0, 1);

      if (pressureHpa > 1020) {
        lcd->print("FAIR_&_DRY");
      } else if (pressureHpa > 1010 && pressureHpa <= 1020) {
        lcd->print("NORMAL");
      } else if (pressureHpa > 1000 && pressureHpa <= 1010) {
        lcd->print("WET&CLOUDY");
      } else if (pressureHpa > 990 && pressureHpa <= 1000) {
        lcd->print("GET_WET_SOON");
      } else {
        lcd->print("APOCALYPSE");
      }
      break;
    }

    case 4: {
      lcd->setCursor(0, 0);
      lcd->print("Future_Alarm)");
      break;
    }
  }
}

void setup() {
  Serial.begin(115200);
  Wire.begin(I2C_SDA, I2C_SCL);

  // ---- LCD init + «bebra» як у твоєму коді ----
  uint8_t addr = detectLcdAddr();
  if (addr == 0) {
    Serial.println("LCD not found on 0x27/0x3F. Check wiring/voltage.");
  } else {
    lcd = new LiquidCrystal_I2C(addr, 16, 2);
    lcd->init();
    lcd->backlight();
    lcd->setCursor(0, 0);
    lcd->print("I love hot bebra)");
    delay(3000);
    lcd->setCursor(0, 1);
    lcd->print("And cold bebra");
    delay(3000);
    lcd->clear();
  }

  // ---- Сенсорна кнопка ----
  pinMode(touchBtn, INPUT); // TTP223 зазвичай дає HIGH при торканні

  // ---- BME280 ----
  if (!bme.begin(0x76)) {
    if (lcd) { lcd->setCursor(0,0); lcd->print("WHERE BME280?)"); }
    Serial.println("BME280 not found at 0x76. Trying 0x77...");
    if (!bme.begin(0x77)) {
      Serial.println("No BME280 at 0x76/0x77.");
      //while (1) delay(100);
    }
  }

  fillPressureBufferOnce();   // заповнимо масив стартовими значеннями
  ShowScreen(screenIndex);    // намалюємо перший екран
  // WiFi
  Serial.print("Connecting to WiFi:");
  Serial.println(ssid);
  WiFi.begin(ssid, password);

  //wait 15s for connection
  int waitTime = 0;
  while (WiFi.status() != WL_CONNECTED && waitTime < 15)
  {
    delay(1000);
    Serial.print(".");
    waitTime++;
  }

  Serial.println();
  if(WiFi.status() == WL_CONNECTED)
  {
    Serial.println("WiFi connected");
    Serial.println("IP CONNECTED:");
    Serial.println(WiFi.localIP());
  }
  else{
    Serial.println("Connection failed...");
  }
  // Web server
  server.on("/", []() { server.send_P(200, "text/html", MAIN_page); });
  server.on("/data.json", handleData);
  server.begin();

}

void loop() {
  // Обробка натискання сенсорної кнопки (перемикання екранів)
  bool touchState = digitalRead(touchBtn);
  if (touchState == HIGH && !lastTouchState) {
    screenIndex++;
    if (screenIndex > (int)screenCount) screenIndex = 1;
    dateDrawn = false;
    if (lcd) lcd->clear();
    ShowScreen(screenIndex);
    lastScreenUpdate = millis();
  }
  lastTouchState = touchState;

  // Екран 1 (Uptime) – оновлювати щосекунди
  if (screenIndex == 1 && millis() - lastTimeUpdate >= 1000) {
    ShowScreen(screenIndex);
    lastTimeUpdate = millis();
  }
  // Інші екрани — раз на 10 секунд (щоб «живіти»)
  else if (screenIndex != 1 && millis() - lastScreenUpdate >= screenDelay) {
    ShowScreen(screenIndex);
    lastScreenUpdate = millis();
  }

  // Оновлення статистики тиску (для «прогнозу»)
  Statistic();
  server.handleClient();
  delay(5);
}