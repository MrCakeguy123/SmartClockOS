# SmartClockOS

A beautiful, feature-rich smart clock for ESP32 with real-time weather, automatic brightness control, and Wi-Fi connectivity. Designed for the ESP32-3248S035C 3.5" capacitive-touch display module with smooth LVGL animations and modern UI.

![ESP-IDF](https://img.shields.io/badge/ESP--IDF-v5.x-blue) ![Platform](https://img.shields.io/badge/platform-ESP32-green)

## ✨ New Features (Latest Update)

### 🌤️ Real Weather Integration
- **Live weather data** from Open-Meteo API (no API key required!)
- **Comprehensive info:** Temperature, feels like, high/low, sunrise/sunset
- **Weather icons** with 20+ conditions (☀ ☁ ☔ ❄ ⚡)
- **Beautiful weather card** UI at bottom of display
- **5-minute auto-updates** with offline fallback

### ⚡ Pre-configured for Your City
- Latitude: 40.7128, Longitude: -74.0060
- Eastern Time Zone with automatic DST
- Ready to flash and use immediately!

## Highlights
- **ESP-IDF v5.x** with LVGL 8.x for polished, animated clock face
- **Wi-Fi SNTP sync** for accurate timekeeping
- **Real weather API** integration with HTTPS
- **Smart power management** (auto-dim, sleep modes)
- **Easy configuration** via simple header file
- **OTA-ready** architecture

## 📚 Documentation

**New users start here:**

| Document | Description |
|----------|-------------|
| **[FLASH_QUICK_START.md](FLASH_QUICK_START.md)** | ⚡ Quick 3-step flash guide |
| **[SETUP_GUIDE.md](SETUP_GUIDE.md)** | 📖 Complete setup instructions |
| **[PRE_FLIGHT_CHECKLIST.md](PRE_FLIGHT_CHECKLIST.md)** | ✅ Get ready before ESP32 arrives |
| **[PORTING_GUIDE.md](PORTING_GUIDE.md)** | 🔧 Technical feature documentation |

## 🚀 Quick Start (3 Steps)

### 1. Install ESP-IDF
Run `install_esp_idf.bat` or download from: https://dl.espressif.com/dl/esp-idf/

### 2. Build Firmware
```cmd
cd firmware
idf.py build
```

### 3. Flash ESP32 (when it arrives)
```cmd
idf.py -p auto flash monitor
```

**See [FLASH_QUICK_START.md](FLASH_QUICK_START.md) for detailed instructions.**

## ⚙️ Configuration

All settings in `firmware/main/config.h`:

```c
// Location (pre-set for Your City)
#define WEATHER_LAT 40.7128
#define WEATHER_LON -74.0060
#define LOCATION_NAME "Your City"

// Timezone (Eastern Time)
#define TIMEZONE_STRING "EST5EDT,M3.2.0,M11.1.0"

// Power management
#define AUTO_DIM_TIMEOUT_SEC 30
#define AUTO_BLANK_TIMEOUT_SEC 60
```

## 🏗️ Repository Layout

```
SmartClockOS/
├── firmware/main/
│   ├── main.c              # Application entry point
│   ├── ui_shell.c          # LVGL UI with weather card
│   ├── weather_service.c   # Open-Meteo API integration
│   ├── network_manager.c   # Wi-Fi management
│   ├── config.h            # ← Your configuration here
│   └── ...
├── SETUP_GUIDE.md          # Full setup guide
├── FLASH_QUICK_START.md    # Quick flash guide
└── README.md               # This file
```

## 🎨 UI Features

- **Large time display** with smooth scale animation
- **Weather card** with icon, temperature, high/low, feels like
- **Sunrise/sunset times** displayed on right side
- **Day and location** shown below time
- **Gradient background** with modern styling
- **Auto-brightness** with configurable dimming
- **Settings panel** for auto-dim and deep sleep toggles
