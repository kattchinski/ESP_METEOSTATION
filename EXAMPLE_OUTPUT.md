# Example Output

This is what you should see in the serial monitor when the meteostation is running:

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

=================================
Date: 2024-10-11 | Time: 12:30:02
Temperature: 22.46 °C
Pressure:    1013.26 hPa
Altitude:    -0.09 m
=================================

=================================
Date: 2024-10-11 | Time: 12:30:04
Temperature: 22.45 °C
Pressure:    1013.24 hPa
Altitude:    0.09 m
=================================
```

## Data Explanation

### Date and Time
- Format: YYYY-MM-DD | HH:MM:SS
- Source: DS1302 Real-Time Clock
- Updated every reading cycle

### Temperature
- Unit: Degrees Celsius (°C)
- Source: BMP280 sensor
- Accuracy: ±1°C typical
- Resolution: 0.01°C

### Pressure
- Unit: Hectopascals (hPa) or millibars (mb)
- Source: BMP280 sensor
- Accuracy: ±1 hPa typical
- Standard sea level pressure: 1013.25 hPa

### Altitude
- Unit: Meters (m)
- Calculated from pressure using standard atmospheric model
- Reference: Sea level (1013.25 hPa)
- Note: This is relative altitude, not absolute elevation

## Reading Frequency

The sensors are read every **2 seconds** by default. This can be changed by modifying the `readInterval` constant in `src/main.cpp`:

```cpp
const unsigned long readInterval = 2000; // milliseconds
```

## Troubleshooting Output

### If BMP280 is not detected:
```
ESP Meteostation Starting...
=================================
ERROR: Could not find BMP280 sensor!
Check wiring and I2C address (0x76 or 0x77)
```
**Solution**: Check wiring and try changing `bmp.begin(0x76)` to `bmp.begin(0x77)` in the code.

### If no output appears:
- Check serial monitor baud rate is set to **115200**
- Check USB connection
- Press the reset button on the ESP8266
