#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_BMP280.h>
#include <DS1302.h>

// DS1302 RTC pins
#define DS1302_CLK_PIN D5  // GPIO14
#define DS1302_DAT_PIN D6  // GPIO12
#define DS1302_RST_PIN D7  // GPIO13

// BMP280 I2C pins (default for ESP8266)
// SDA -> D2 (GPIO4)
// SCL -> D1 (GPIO5)

// Create DS1302 instance
DS1302 rtc(DS1302_RST_PIN, DS1302_DAT_PIN, DS1302_CLK_PIN);

// Create BMP280 instance
Adafruit_BMP280 bmp; // I2C

// Variables to store sensor data
float temperature = 0.0;
float pressure = 0.0;
float altitude = 0.0;

// Timing variables
unsigned long lastReadTime = 0;
const unsigned long readInterval = 2000; // Read sensors every 2 seconds

void setup() {
  // Initialize serial communication
  Serial.begin(115200);
  delay(1000);
  Serial.println();
  Serial.println("ESP Meteostation Starting...");
  Serial.println("=================================");

  // Initialize I2C for BMP280
  Wire.begin();

  // Initialize BMP280
  if (!bmp.begin(0x76)) {
    Serial.println("ERROR: Could not find BMP280 sensor!");
    Serial.println("Check wiring and I2C address (0x76 or 0x77)");
    while (1) delay(100);
  }

  // Configure BMP280 for weather monitoring
  bmp.setSampling(Adafruit_BMP280::MODE_NORMAL,     /* Operating Mode */
                  Adafruit_BMP280::SAMPLING_X2,     /* Temp. oversampling */
                  Adafruit_BMP280::SAMPLING_X16,    /* Pressure oversampling */
                  Adafruit_BMP280::FILTER_X16,      /* Filtering */
                  Adafruit_BMP280::STANDBY_MS_500); /* Standby time */

  Serial.println("BMP280 sensor initialized successfully!");
  
  // Initialize DS1302 RTC
  rtc.halt(false);
  rtc.writeProtect(false);

  // Check if RTC is running
  Serial.println("DS1302 RTC initialized!");
  
  // Uncomment the following lines to set the time (do this once, then comment out)
  // Time t(2024, 10, 11, 12, 30, 0, Time::kThursday);
  // rtc.time(t);
  // Serial.println("RTC time has been set!");

  Serial.println("=================================");
  Serial.println("System Ready!");
  Serial.println();
}

void printDateTime() {
  Time t = rtc.time();
  
  char dateStr[20];
  char timeStr[20];
  
  snprintf(dateStr, sizeof(dateStr), "%04d-%02d-%02d", 
           t.yr, t.mon, t.date);
  snprintf(timeStr, sizeof(timeStr), "%02d:%02d:%02d", 
           t.hr, t.min, t.sec);
  
  Serial.print("Date: ");
  Serial.print(dateStr);
  Serial.print(" | Time: ");
  Serial.print(timeStr);
}

void readBMP280() {
  temperature = bmp.readTemperature();
  pressure = bmp.readPressure() / 100.0F; // Convert Pa to hPa
  altitude = bmp.readAltitude(1013.25);   // Sea level pressure
}

void printSensorData() {
  Serial.println("=================================");
  
  // Print date and time from RTC
  printDateTime();
  Serial.println();
  
  // Print BMP280 data
  Serial.print("Temperature: ");
  Serial.print(temperature, 2);
  Serial.println(" °C");
  
  Serial.print("Pressure:    ");
  Serial.print(pressure, 2);
  Serial.println(" hPa");
  
  Serial.print("Altitude:    ");
  Serial.print(altitude, 2);
  Serial.println(" m");
  
  Serial.println("=================================");
  Serial.println();
}

void loop() {
  unsigned long currentTime = millis();
  
  // Read sensors at specified interval
  if (currentTime - lastReadTime >= readInterval) {
    lastReadTime = currentTime;
    
    // Read sensor data
    readBMP280();
    
    // Print data to serial
    printSensorData();
  }
  
  // Small delay to prevent excessive CPU usage
  delay(10);
}
