# ESP Meteostation Wiring Diagram

```
ESP8266 Board (ESP-12E / NodeMCU)
==================================

DS1302 RTC Module:
------------------
DS1302 VCC  →  ESP8266 3.3V
DS1302 GND  →  ESP8266 GND
DS1302 CLK  →  ESP8266 D5 (GPIO14)
DS1302 DAT  →  ESP8266 D6 (GPIO12)
DS1302 RST  →  ESP8266 D7 (GPIO13)

BMP280 Sensor (I2C):
--------------------
BMP280 VCC  →  ESP8266 3.3V
BMP280 GND  →  ESP8266 GND
BMP280 SDA  →  ESP8266 D2 (GPIO4)
BMP280 SCL  →  ESP8266 D1 (GPIO5)

Power:
------
Power the ESP8266 via USB or external 5V supply

Serial Connection:
------------------
Connect ESP8266 to computer via USB
Baud rate: 115200
```

## Pin Mapping Reference

| Component | Pin | ESP8266 GPIO | Description |
|-----------|-----|--------------|-------------|
| DS1302    | CLK | GPIO14 (D5)  | Clock       |
| DS1302    | DAT | GPIO12 (D6)  | Data        |
| DS1302    | RST | GPIO13 (D7)  | Reset       |
| BMP280    | SDA | GPIO4 (D2)   | I2C Data    |
| BMP280    | SCL | GPIO5 (D1)   | I2C Clock   |

## Notes

1. **Power**: Both sensors should be powered with 3.3V, not 5V
2. **I2C Address**: BMP280 typically uses address 0x76, some modules use 0x77
3. **RTC Battery**: DS1302 requires a CR2032 battery to keep time when power is off
4. **Pull-up resistors**: I2C bus may need 4.7kΩ pull-up resistors on SDA and SCL if not already on the sensor modules
