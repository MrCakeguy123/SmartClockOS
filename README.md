# SmartClockOS

SmartClockOS is a modern ESP32 firmware designed for the ESP32-3248S035C 3.5" capacitive-touch display module. It targets a Wi-Fi-synced smart clock with smooth LVGL animations, location-aware gradients, and OTA-ready firmware structure.

## Highlights
- ESP-IDF-first design with LVGL for a polished, animated clock face.
- Wi-Fi SNTP synchronization for accurate timekeeping and weather polling.
- Location-aware theming (timezone/sunrise/weather stubs ready for integration).
- OTA-ready architecture (dual-slot approach recommended).

## Repository Layout
- `firmware/main/main.c` – ESP-IDF entry point, Wi-Fi + SNTP bootstrap, LVGL UI stub.
- `docs/architecture.md` – High-level design and roadmap.

## Building
1. Install [ESP-IDF](https://docs.espressif.com/projects/esp-idf/en/latest/esp32/get-started/index.html) v5.x.
2. Configure your display/touch drivers and Wi-Fi credentials in `firmware/main/main.c`.
3. From the `firmware` directory, run the usual ESP-IDF workflow:
   ```bash
   idf.py set-target esp32
   idf.py menuconfig
   idf.py flash monitor
   ```

## UI Direction
The default face uses large typography, gradient backgrounds, and subtle scale animations on the time label. A dedicated weather line sits above the bottom edge so the user always sees current conditions. Swipes can switch faces, and a pull-down reveals quick settings. Extend `app_create_clock_ui` with additional LVGL scenes for richer motion.
