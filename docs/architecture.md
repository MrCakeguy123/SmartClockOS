# SmartClockOS Architecture

SmartClockOS targets the ESP32-3248S035C capacitive touch display (480x320, ST7796 controller) as a modern smart clock with smooth animations, Wi-Fi time sync, and location-aware behavior. The system is designed to be lean enough for the ESP32 but still provide a polished UX using LVGL.

## Goals
- **Accurate time** via Wi-Fi SNTP with periodic resync and drift monitoring.
- **Location-aware** clock faces (timezone, weather-ready stubs, sunrise/sunset cues, current conditions).
- **Modern UI** driven by LVGL, targeting 60 FPS animations on the 480x320 panel.
- **Resilient updates** with OTA capability and rollback-ready partitions (using ESP-IDF OTA APIs).
- **Low-power aware**: screen dimming, sleep/wake hooks for RTC/touch events.

## Hardware Profile
- **MCU**: ESP32 (ESP32-3248S035C module) with built-in Wi-Fi/BLE.
- **Display**: 3.5" 480x320 TFT, ST7796 controller, capacitive touch.
- **Storage**: External flash for firmware + SPIFFS/LittleFS for themes/config.
- **Peripherals**: RTC, touch controller (I2C), optional buzzer.

## Software Stack
- **Base SDK**: ESP-IDF (v5.x recommended).
- **UI**: LVGL 8.x with GPU-less theme tuned for 480x320.
- **Networking**: Wi-Fi station mode + captive portal onboarding; SNTP for time sync; HTTP client for weather.
- **Config**: NVS for credentials and preferences; JSON profiles in SPIFFS/LittleFS (includes weather API key/units).
- **OTA**: Dual-slot OTA with checksum/rollback.

## Runtime Components
- **Boot**: Initialize NVS, network stack, display, touch, and LVGL tick.
- **Network Manager**: Wi-Fi join with retry/backoff; captive portal AP fallback.
- **Time Service**: SNTP init + periodic resync; drift logging; timezone updates.
- **Location Service**: Geo source abstraction (IP-lookup, manual lat/long) feeding timezone/sun data and weather queries; currently stubbed.
- **Weather Service**: Periodic HTTP fetch (e.g., OpenWeather) mapped into simple condition/temperature strings cached for UI.
- **UI Shell**: Scene manager that swaps between clock faces, settings, and onboarding flows with LVGL animations.
- **Power Manager**: Dim/blank screen on idle, wake on touch/RTC alarm; optional deep sleep.

## UI Concepts
- **Default face**: Large typography, dynamic gradient background based on time-of-day, smooth minute/second transitions, and inline weather summary.
- **Animations**: LVGL style/opacity transforms on second tick; parallax background layers; slide-in panels for settings.
- **Touch UX**: Horizontal swipe to switch faces, vertical pull to reveal quick settings (Wi-Fi status, brightness).

## Data Flows
1. **On boot**: Initialize services → attempt Wi-Fi join → start SNTP → show onboarding if not provisioned.
2. **Timekeeping**: SNTP sets system time → timezone offset applied → LVGL clock updates every second.
3. **Location**: When Wi-Fi available, fetch geo/timezone (stub) → persist to NVS → UI updates gradients/sunrise cues.
4. **OTA**: User triggers from settings → download to inactive slot → swap on reboot with rollback flag cleared post-boot.

## Files & Directories
- `firmware/main/main.c`: ESP-IDF entry with service initialization and LVGL UI bootstrap.
- `docs/architecture.md`: This document, high-level design and expectations.
- `README.md`: Quickstart and repo overview.

## Next Steps
- Flesh out Wi-Fi onboarding screen and captive portal handler.
- Add timezone lookup provider (e.g., HTTP to public API) gated behind user consent.
- Implement theme assets and animation timelines for the clock faces.
- Integrate OTA workflow and partition table.
