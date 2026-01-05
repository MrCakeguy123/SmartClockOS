# Emulator Features Ported to Main OS

This document describes all the features that have been ported from the LVGL emulator to the main SmartClock OS firmware.

## Summary of Changes

All features from the Visual Studio simulator have been successfully ported to the ESP32 firmware, including:

1. **Real Weather API Integration** - Open-Meteo API with HTTPS support
2. **Enhanced Weather Data** - Temperature, feels like, high/low, sunrise/sunset
3. **WMO Weather Code Mapping** - 20+ weather conditions with icons
4. **Improved UI** - Weather card with icons, details, and sun times
5. **Easy Configuration** - Simple config file for location and settings

## Features Ported

### 1. Weather Service Enhancements

#### Enhanced Data Structure
The weather data structure has been expanded from a simple description string to a comprehensive weather object:

**Before:**
```c
typedef struct {
    char description[48];  // "22°C • Clear Skies"
} weather_data_t;
```

**After:**
```c
typedef struct {
    double temp_f;       // Current temperature in Fahrenheit
    double feels_like_f; // Apparent temperature
    double high_f;       // Today's high
    double low_f;        // Today's low
    int weather_code;    // WMO weather code
    char condition[32];  // Weather description
    char sunrise[8];     // e.g., "7:15 AM"
    char sunset[8];      // e.g., "5:42 PM"
} weather_data_t;
```

#### Real Weather API Integration
- **API Provider:** Open-Meteo (no API key required)
- **Protocol:** HTTPS with certificate bundle validation
- **Data Fetched:**
  - Current temperature (Fahrenheit)
  - Apparent temperature (feels like)
  - Daily high and low temperatures
  - Sunrise and sunset times
  - WMO weather condition code
- **Implementation:** ESP-IDF HTTP client with automatic timezone detection

**Files Modified:**
- `firmware/main/weather_service.c` - Complete rewrite with HTTP client
- `firmware/main/weather_service.h` - Updated data structure

### 2. JSON Parsing

Implemented lightweight JSON parsing functions (no external library needed):

- `parse_json_double()` - Parse floating-point values
- `parse_json_int()` - Parse integer values
- `parse_sun_time()` - Parse ISO 8601 time strings from arrays

These functions handle both direct values and array formats from the Open-Meteo API.

### 3. WMO Weather Code Mapping

Added comprehensive weather condition mapping based on World Meteorological Organization codes:

| Code Range | Condition |
|------------|-----------|
| 0 | Clear |
| 1 | Mostly Clear |
| 2 | Partly Cloudy |
| 3 | Overcast |
| 45, 48 | Foggy |
| 51-57 | Drizzle / Freezing Drizzle |
| 61-67 | Rain / Freezing Rain |
| 71-77 | Snow / Snow Grains |
| 80-82 | Showers |
| 85-86 | Snow Showers |
| 95 | Thunderstorm |
| 96, 99 | Thunderstorm with Hail |

**Files Modified:**
- `firmware/main/weather_service.c:31-80` - WMO code mapping function

### 4. Enhanced UI Display

#### Weather Card Design
Replaced the simple single-line weather label with a comprehensive weather card:

**Layout:**
```
┌─────────────────────────────────────────────┐
│ ☀  72°F • Clear             Rise: 7:15 AM   │
│     H:75° L:65° • Feels 70°  Set: 5:42 PM   │
└─────────────────────────────────────────────┘
```

**Features:**
- Weather icon (Unicode symbols: ☀ ☁ ☔ ❄ ⚡)
- Current temperature and condition
- High/low temperatures
- "Feels like" temperature
- Sunrise and sunset times
- Styled card with gradient colors

**Files Modified:**
- `firmware/main/ui_shell.c:355-392` - Weather card UI creation
- `firmware/main/ui_shell.c:495-527` - Weather update function
- `firmware/main/ui_shell.c:19-60` - Weather icon mapping
- `firmware/main/ui_shell.h:33` - New function declaration

#### Weather Icon Mapping
Unicode weather symbols mapped to WMO codes:
- ☀ (U+2600) - Clear
- ⛅ (U+26C5) - Partly Cloudy
- ☁ (U+2601) - Cloudy
- 🌫 (U+1F32B) - Fog
- ☔ (U+2614) - Rain
- ❄ (U+2744) - Snow
- ⚡ (U+26A1) - Thunderstorm

### 5. Location Configuration

Created a centralized configuration system for easy customization:

**New File:** `firmware/main/config.h`

**Configuration Options:**
```c
// Weather location (decimal degrees)
#define WEATHER_LAT 38.6820
#define WEATHER_LON -84.5894

// Display location name
#define LOCATION_NAME "Paris"

// Power management timeouts
#define AUTO_DIM_TIMEOUT_SEC 30
#define AUTO_BLANK_TIMEOUT_SEC 60
#define DEEP_SLEEP_TIMEOUT_SEC 600

// Night mode hours
#define NIGHT_MODE_START_HOUR 22  // 10 PM
#define NIGHT_MODE_END_HOUR 6     // 6 AM

// Wi-Fi credentials (optional)
#define WIFI_SSID ""
#define WIFI_PASSWORD ""

// NTP configuration
#define NTP_SERVER "pool.ntp.org"
#define TIMEZONE_STRING "EST5EDT,M3.2.0,M11.1.0"
```

**How to Configure:**

1. **Find Your Coordinates:**
   - Go to https://www.google.com/maps
   - Right-click your location
   - Click the coordinates to copy them
   - Update `WEATHER_LAT` and `WEATHER_LON` in `config.h`

2. **Set Your Location Name:**
   - Update `LOCATION_NAME` in `config.h`
   - This appears in the UI: "Mon • Paris"

3. **Configure Timezone:**
   - Find your POSIX timezone string: https://github.com/nayarsystems/posix_tz_db/blob/master/zones.csv
   - Update `TIMEZONE_STRING` in `config.h`

**Files Created:**
- `firmware/main/config.h` - Centralized configuration

**Files Modified:**
- `firmware/main/weather_service.c:2` - Include config.h
- `firmware/main/ui_shell.c:2` - Include config.h
- `firmware/main/ui_shell.c:170` - Use LOCATION_NAME

### 6. Build Configuration

Updated ESP-IDF build system to include required dependencies:

**Dependencies Added:**
- `esp_http_client` - HTTPS client with SSL/TLS support

**Files Modified:**
- `firmware/main/CMakeLists.txt:4` - Added esp_http_client to REQUIRES

## Testing

### Prerequisites
- ESP-IDF 5.x installed
- ESP32 development board
- Wi-Fi network access
- Internet connection for weather API

### Build Instructions

1. **Navigate to firmware directory:**
   ```bash
   cd SmartClockOS/firmware
   ```

2. **Configure the project:**
   ```bash
   idf.py menuconfig
   ```

3. **Build the firmware:**
   ```bash
   idf.py build
   ```

4. **Flash to device:**
   ```bash
   idf.py -p COMx flash monitor
   ```
   (Replace `COMx` with your serial port)

### Expected Behavior

1. **Boot Sequence:**
   - Loading screen with progress bar
   - "Loading weather service" at 45%
   - Transition to clock UI at 100%

2. **Network Connection:**
   - "Connecting to Wi-Fi" status message
   - Automatic SNTP time synchronization
   - Immediate weather update on connection

3. **Weather Display:**
   - Weather card appears at bottom of screen
   - Shows "Loading..." initially
   - Updates with real weather data after network connection
   - Displays temperature, condition, high/low, feels like
   - Shows sunrise and sunset times
   - Updates every 5 minutes (300 seconds)

4. **Offline Behavior:**
   - Shows "Offline" if weather fetch fails
   - Sunrise/sunset shows "--:--"
   - Retries on next update interval

### Troubleshooting

#### Weather Shows "Offline"
- Check Wi-Fi connection status
- Verify internet access
- Check ESP logs for HTTP errors:
  ```
  I (12345) weather_service: HTTP Status = 200, content_length = 1234
  ```

#### Wrong Location
- Verify `WEATHER_LAT` and `WEATHER_LON` in `config.h`
- Rebuild and flash firmware

#### Build Errors
- Ensure ESP-IDF 5.x is installed
- Run `idf.py fullclean` then `idf.py build`
- Check that esp_http_client component is available

#### Certificate Validation Errors
- Ensure `esp_crt_bundle_attach` is enabled
- Check that ESP-IDF has certificate bundle support

## API Details

### Open-Meteo API

**Endpoint:**
```
https://api.open-meteo.com/v1/forecast
```

**Parameters:**
- `latitude` - Location latitude (decimal degrees)
- `longitude` - Location longitude (decimal degrees)
- `current` - Current weather variables (temperature_2m, apparent_temperature, weather_code)
- `daily` - Daily forecast variables (temperature_2m_max, temperature_2m_min, sunrise, sunset)
- `temperature_unit` - fahrenheit
- `timezone` - auto (uses location timezone)
- `forecast_days` - 1

**Response Format:** JSON
```json
{
  "current": {
    "temperature_2m": 72.5,
    "apparent_temperature": 70.2,
    "weather_code": 0
  },
  "daily": {
    "temperature_2m_max": [75.0],
    "temperature_2m_min": [65.0],
    "sunrise": ["2026-01-04T07:15"],
    "sunset": ["2026-01-04T17:42"]
  }
}
```

**Rate Limits:** None (free tier)
**Attribution:** Not required but appreciated

## File Changes Summary

### Modified Files
1. `firmware/main/weather_service.c` - Complete rewrite with HTTP client and JSON parsing
2. `firmware/main/weather_service.h` - Enhanced weather_data_t structure
3. `firmware/main/ui_shell.c` - Weather card UI and display logic
4. `firmware/main/ui_shell.h` - New ui_shell_update_weather_data() function
5. `firmware/main/main.c` - Updated weather callback to use new data structure
6. `firmware/main/CMakeLists.txt` - Added esp_http_client dependency

### New Files
1. `firmware/main/config.h` - Centralized configuration file

### Lines Changed
- **weather_service.c:** 33 lines → 285 lines (+252)
- **weather_service.h:** 28 lines → 28 lines (structure changed)
- **ui_shell.c:** 443 lines → 543 lines (+100)
- **ui_shell.h:** 38 lines → 39 lines (+1)
- **main.c:** 1 line changed (callback)
- **CMakeLists.txt:** 1 dependency added
- **config.h:** 115 lines (new file)

**Total Changes:** ~470 lines added/modified

## Migration Notes

### Backwards Compatibility
- Old `ui_shell_update_weather(const char *text)` function still exists for compatibility
- New code should use `ui_shell_update_weather_data(const weather_data_t *data)`

### Performance Impact
- HTTP request adds ~2-5 seconds on first weather update
- Subsequent updates cached for 5 minutes
- Minimal memory overhead (~4KB for HTTP buffer)
- No impact on touch responsiveness or display refresh

### Security Considerations
- Uses HTTPS with certificate bundle validation
- No API key required (Open-Meteo is free)
- No sensitive data transmitted
- Location coordinates are public information

## Future Enhancements

Potential improvements for future versions:

1. **NVS Storage** - Save location in flash memory for easy reconfiguration
2. **Location Auto-detection** - Use IP geolocation API
3. **Extended Forecast** - Show 3-day or 7-day forecast
4. **Weather Alerts** - Display severe weather warnings
5. **Multiple Locations** - Support for multiple saved locations
6. **Temperature Units** - Toggle between Fahrenheit and Celsius
7. **Weather Graphs** - Temperature trends throughout the day
8. **Custom Icons** - Add custom weather icon font
9. **Alternative APIs** - Support for OpenWeatherMap, Weather.gov, etc.
10. **Offline Cache** - Store last weather data in NVS for offline display

## Credits

- **Original Emulator:** SmartClock OS Visual Studio Simulator
- **Weather API:** Open-Meteo (https://open-meteo.com/)
- **WMO Codes:** World Meteorological Organization
- **Framework:** ESP-IDF, LVGL

## License

Same license as SmartClock OS project.
