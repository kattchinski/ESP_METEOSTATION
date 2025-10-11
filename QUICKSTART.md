# Quick Start Guide

Get your ESP Meteostation up and running in 5 minutes!

## Prerequisites

- ESP8266 board (NodeMCU or ESP-12E)
- DS1302 RTC module with battery
- BMP280 sensor module
- USB cable
- Jumper wires

## Step 1: Wire the Components

Follow the [WIRING.md](WIRING.md) guide to connect:
- DS1302 to pins D5, D6, D7
- BMP280 to pins D1 (SCL) and D2 (SDA)
- Both sensors to 3.3V and GND

## Step 2: Install PlatformIO

Choose one:

**Option A - VS Code Extension (Recommended):**
1. Install [Visual Studio Code](https://code.visualstudio.com/)
2. Install the PlatformIO IDE extension from the extensions marketplace

**Option B - Command Line:**
```bash
pip install platformio
```

## Step 3: Clone and Open Project

```bash
git clone https://github.com/kattchinski/ESP_METEOSTATION.git
cd ESP_METEOSTATION
```

In VS Code: File → Open Folder → Select ESP_METEOSTATION

## Step 4: Set the RTC Time (First Time Only)

1. Open `src/main.cpp`
2. Find these lines (around line 65):
   ```cpp
   // Time t(2024, 10, 11, 12, 30, 0, Time::kThursday);
   // rtc.time(t);
   ```
3. Uncomment and set current date/time
4. Upload to board
5. Comment them out again and re-upload

## Step 5: Upload to ESP8266

**Via VS Code:**
- Click the Upload button (→) in the PlatformIO toolbar

**Via Command Line:**
```bash
pio run -t upload
```

## Step 6: View Serial Output

**Via VS Code:**
- Click the Serial Monitor button in the PlatformIO toolbar

**Via Command Line:**
```bash
pio device monitor
```

You should see output like:
```
ESP Meteostation Starting...
=================================
BMP280 sensor initialized successfully!
DS1302 RTC initialized!
=================================
System Ready!

=================================
Date: 2024-10-11 | Time: 12:30:00
Temperature: 22.45 °C
Pressure:    1013.25 hPa
Altitude:    0.00 m
=================================
```

## Troubleshooting

**BMP280 not found?**
- Check I2C wiring (SDA/SCL)
- Try changing `bmp.begin(0x76)` to `bmp.begin(0x77)` in main.cpp

**No serial output?**
- Set baud rate to 115200
- Press reset button on ESP8266
- Check USB connection

**Time not keeping?**
- Ensure DS1302 has a CR2032 battery installed
- Re-run the RTC time setting procedure

## What's Next?

- Monitor your weather data over time
- Add WiFi connectivity to publish data online
- Connect an OLED display for standalone operation
- Log data to SD card for historical tracking

See [README.md](README.md) for more detailed information.
