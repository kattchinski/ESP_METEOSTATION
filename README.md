# ESP_METEOSTATION

Meteostation to monitor pressure, temperature on ds1302, bmp280. Arduino framework

## Features

- Real-time clock (DS1302) for accurate timestamping
- Temperature and pressure monitoring (BMP280 sensor)
- Altitude calculation based on atmospheric pressure
- Serial output of sensor readings every 2 seconds
- Optimized for weather monitoring applications

## Hardware Requirements

- ESP8266 board (ESP-12E/F, NodeMCU, or similar)
- DS1302 Real-Time Clock module
- BMP280 Barometric Pressure and Temperature sensor
- Jumper wires

## Wiring

### DS1302 RTC Module
- VCC → 3.3V
- GND → GND
- CLK → D5 (GPIO14)
- DAT → D6 (GPIO12)
- RST → D7 (GPIO13)

### BMP280 Sensor (I2C)
- VCC → 3.3V
- GND → GND
- SDA → D2 (GPIO4)
- SCL → D1 (GPIO5)

## Software Requirements

- [PlatformIO](https://platformio.org/) installed (VS Code extension or CLI)
- Or Arduino IDE with ESP8266 board support

## Installation

1. Clone this repository:
   ```bash
   git clone https://github.com/kattchinski/ESP_METEOSTATION.git
   cd ESP_METEOSTATION
   ```

2. Open the project in PlatformIO (VS Code) or compile using PlatformIO CLI

3. Build and upload:
   ```bash
   pio run -t upload
   ```

4. Monitor serial output:
   ```bash
   pio device monitor
   ```

## First Time Setup

On first run, you need to set the RTC time. In `src/main.cpp`, uncomment these lines in the `setup()` function:

```cpp
Time t(2024, 10, 11, 12, 30, 0, Time::kThursday);
rtc.time(t);
Serial.println("RTC time has been set!");
```

Set the correct date and time, upload the code, then comment out these lines again and re-upload.

## Usage

Once running, the meteostation will:
1. Display initialization messages on serial console (115200 baud)
2. Read sensor data every 2 seconds
3. Output formatted data including:
   - Current date and time from RTC
   - Temperature in °C
   - Atmospheric pressure in hPa
   - Calculated altitude in meters

### Example Output

```
=================================
Date: 2024-10-11 | Time: 12:30:45
Temperature: 22.45 °C
Pressure:    1013.25 hPa
Altitude:    0.00 m
=================================
```

## Dependencies

The following libraries are automatically installed by PlatformIO:
- Adafruit BMP280 Library (^2.6.8)
- Adafruit Unified Sensor (^1.1.14)
- Time Library (^1.6.1)
- DS1302 Library (^1.3.1)

## Troubleshooting

### BMP280 not found
- Check I2C wiring (SDA and SCL)
- Verify sensor I2C address (usually 0x76 or 0x77)
- Try changing the address in code: `bmp.begin(0x77)`

### RTC time not accurate
- Ensure DS1302 module has a backup battery installed
- Verify wiring connections
- Re-run the time setting procedure

### Serial output garbled
- Ensure serial monitor is set to 115200 baud
- Check USB cable and connection

## License

MIT License

## Author

kattchinski
