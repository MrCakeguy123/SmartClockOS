# SmartClock OS - Pre-Flight Checklist

Complete this checklist **before** your ESP32 arrives so you're ready to flash immediately!

---

## ✅ Software Setup

### ESP-IDF Installation

- [ ] Download ESP-IDF Windows Installer from: https://dl.espressif.com/dl/esp-idf/
- [ ] Run installer (esp-idf-tools-setup-5.x.exe)
- [ ] Installation completed successfully
- [ ] Can open "ESP-IDF 5.x CMD" from Start Menu
- [ ] Run `idf.py --version` shows ESP-IDF v5.x

**How to verify:**
```cmd
# Open "ESP-IDF 5.x CMD" from Start Menu
idf.py --version
# Should show: ESP-IDF v5.x.x
```

---

### Firmware Build Test

- [ ] Open "ESP-IDF 5.x CMD"
- [ ] Navigate to firmware directory
- [ ] Run `idf.py build`
- [ ] Build completes without errors
- [ ] See message: "Project build complete"

**How to test:**
```cmd
cd "C:\Users\logan\Downloads\SmartClock OS Codex\SmartClockOS\firmware"
idf.py build
```

**Expected output:**
```
...
[100%] Linking C executable smartclock.elf
...
Project build complete. To flash, run:
idf.py -p (PORT) flash
```

**If build fails:**
- Check you're using ESP-IDF CMD, not regular CMD
- Run: `idf.py fullclean` then `idf.py build`
- Verify ESP-IDF version is 5.x or newer

---

## ✅ Configuration

### Location Settings

- [ ] Open `firmware/main/config.h`
- [ ] Verify latitude: `38.6820` (Dry Ridge, KY)
- [ ] Verify longitude: `-84.5894`
- [ ] Verify location name: `"Dry Ridge"`
- [ ] Verify timezone: `"EST5EDT,M3.2.0,M11.1.0"` (Eastern Time)

**File location:**
```
C:\Users\logan\Downloads\SmartClock OS Codex\SmartClockOS\firmware\main\config.h
```

**What to check:**
```c
#define WEATHER_LAT 38.6820
#define WEATHER_LON -84.5894
#define LOCATION_NAME "Dry Ridge"
#define TIMEZONE_STRING "EST5EDT,M3.2.0,M11.1.0"
```

---

### Wi-Fi Settings (Optional)

Choose one option:

**Option A: Provisioning Portal (Recommended)**
- [ ] Leave WIFI_SSID and WIFI_PASSWORD empty in config.h
- [ ] ESP32 will create AP "SmartClock-XXXX" on first boot
- [ ] You'll configure Wi-Fi via web browser

**Option B: Hardcode Wi-Fi (Faster Testing)**
- [ ] Edit config.h with your Wi-Fi credentials:
  ```c
  #define WIFI_SSID "YourNetworkName"
  #define WIFI_PASSWORD "YourPassword"
  ```
- [ ] Rebuild firmware: `idf.py build`

---

### Power Management Settings

- [ ] Review auto-dim timeout (default: 30 seconds)
- [ ] Review auto-blank timeout (default: 60 seconds)
- [ ] Review deep sleep timeout (default: 10 minutes)
- [ ] Review night mode hours (default: 10 PM - 6 AM)

**In config.h:**
```c
#define AUTO_DIM_TIMEOUT_SEC 30      // Adjust if needed
#define AUTO_BLANK_TIMEOUT_SEC 60    // Adjust if needed
#define DEEP_SLEEP_TIMEOUT_SEC 600   // Adjust if needed
#define NIGHT_MODE_START_HOUR 22     // 10 PM
#define NIGHT_MODE_END_HOUR 6        // 6 AM
```

---

## ✅ Hardware Preparation

### USB Drivers

- [ ] Download CH340 driver: http://www.wch-ic.com/downloads/CH341SER_EXE.html
- [ ] OR download CP2102 driver: https://www.silabs.com/developers/usb-to-uart-bridge-vcp-drivers
- [ ] Install appropriate driver for your ESP32

**Which driver do I need?**
- Most cheap ESP32 boards use CH340
- Official ESP32 DevKits use CP2102
- You can install both to be safe

---

### USB Cable

- [ ] Have a **data-capable** USB cable (not just charging cable)
- [ ] Cable is USB-A to Micro-USB or USB-C (depending on ESP32)
- [ ] Cable tested with other devices

**How to test:**
- Try connecting phone/device and transferring files
- If file transfer works, cable supports data

---

## ✅ Documentation Review

- [ ] Read `SETUP_GUIDE.md` (full setup guide)
- [ ] Read `FLASH_QUICK_START.md` (quick flash guide)
- [ ] Read `PORTING_GUIDE.md` (feature documentation)
- [ ] Bookmarked ESP-IDF docs: https://docs.espressif.com/projects/esp-idf/

---

## ✅ Tools & Utilities

### Required Tools (Installed)

- [ ] Python 3.8+ installed
- [ ] Git installed
- [ ] ESP-IDF toolchain installed
- [ ] Can run `idf.py` commands

### Useful Tools (Optional)

- [ ] Device Manager open (to find COM port)
- [ ] Terminal emulator (PuTTY, screen, etc.) for serial monitoring
- [ ] Text editor (VS Code, Notepad++, etc.) for code changes

---

## ✅ Troubleshooting Prep

### Common Issues - Solutions Ready

- [ ] Know how to hold BOOT button during flash
- [ ] Know where Device Manager is (Win+X → Device Manager)
- [ ] Know how to exit serial monitor (Ctrl+])
- [ ] Have troubleshooting section bookmarked in SETUP_GUIDE.md

### Emergency Commands

```cmd
# If flash fails, try manual reset:
# 1. Hold BOOT button
# 2. Press RESET button
# 3. Release BOOT button
# 4. Run: idf.py -p COM3 flash

# If everything is broken:
idf.py fullclean
idf.py build
idf.py -p auto erase-flash
idf.py -p auto flash monitor
```

---

## ✅ Final Checks

### Build Verification

- [ ] Firmware built successfully at least once
- [ ] Binary files exist in `build/` directory
- [ ] No build errors or warnings
- [ ] Build completes in reasonable time (2-5 minutes)

**Check build output:**
```cmd
dir "C:\Users\logan\Downloads\SmartClock OS Codex\SmartClockOS\firmware\build"
# Should see: smartclock.bin, bootloader.bin, partition-table.bin
```

---

### Environment Test

- [ ] Can open ESP-IDF CMD
- [ ] Can navigate to firmware directory
- [ ] `idf.py --version` works
- [ ] `idf.py build` works
- [ ] Ready to run `idf.py flash` when ESP32 arrives

---

## 🚀 When ESP32 Arrives

Once all checkboxes above are complete, you're ready! When your ESP32 arrives:

### 3-Step Flash Process

1. **Connect ESP32 to USB**
   - Plug in USB cable
   - Wait for Windows to recognize device

2. **Open ESP-IDF CMD**
   ```cmd
   cd "C:\Users\logan\Downloads\SmartClock OS Codex\SmartClockOS\firmware"
   ```

3. **Flash and Monitor**
   ```cmd
   idf.py -p auto flash monitor
   ```

That's it! Your SmartClock OS will boot up in ~30 seconds.

---

## 📊 Progress Tracker

**Overall Readiness:** `___/6` sections complete

1. Software Setup: `___/2` items
2. Configuration: `___/4` items
3. Hardware Preparation: `___/2` items
4. Documentation Review: `___/4` items
5. Tools & Utilities: `___/2` items
6. Final Checks: `___/2` items

**Goal:** 100% complete before ESP32 arrives

---

## 💡 Pro Tips

### Speed Up First Flash

If you want the fastest possible flash when ESP32 arrives:

1. Build firmware NOW: `idf.py build`
2. When ESP32 arrives, just run: `idf.py -p auto app-flash`
3. Saves 2-3 minutes!

### Test Build Regularly

While waiting for ESP32:
- Make any config changes you want
- Test build after each change
- Fix any build errors now, not when ESP32 arrives

### Prepare Your Workspace

- Clear desk space for ESP32 and display
- Have good lighting for connecting hardware
- Have phone/tablet ready for Wi-Fi provisioning
- Keep this checklist open during first flash

---

## ✅ Ready to Flash!

When all items are checked, you're 100% ready to flash your ESP32 as soon as it arrives!

**Estimated time from unboxing to running clock:** ~5 minutes

Good luck! 🎉
