# SmartClock OS - Flash Quick Start

When your ESP32 arrives, follow these simple steps to get started.

## Prerequisites

✅ ESP-IDF v5.x installed
✅ Firmware built successfully
✅ USB cable (data-capable)
✅ ESP32 connected to computer

---

## Step 1: Install ESP-IDF (One-Time Setup)

### Option A: Automated (Recommended)
1. Double-click `install_esp_idf.bat`
2. Choose option 1 (Windows Installer)
3. Download and run the installer
4. Wait for installation (~10-20 minutes)

### Option B: Download Directly
1. Go to: https://dl.espressif.com/dl/esp-idf/
2. Download: `esp-idf-tools-setup-5.x.exe`
3. Run installer, use default settings
4. Open "ESP-IDF 5.x CMD" from Start Menu

---

## Step 2: Build Firmware (One-Time Setup)

Open "ESP-IDF 5.x CMD" and run:

```cmd
cd "C:\Users\logan\Downloads\SmartClock OS Codex\SmartClockOS\firmware"
idf.py build
```

Wait 2-5 minutes. Look for: **"Project build complete"**

---

## Step 3: Flash ESP32 (When It Arrives)

### Connect ESP32
1. Plug ESP32 into USB port
2. Wait for driver installation

### Flash the Firmware

Open "ESP-IDF 5.x CMD" and run:

```cmd
cd "C:\Users\logan\Downloads\SmartClock OS Codex\SmartClockOS\firmware"
idf.py -p auto flash monitor
```

That's it! 🎉

---

## What You'll See

### During Flash (~30 seconds)
```
Connecting........
Writing at 0x00010000... (10%)
Writing at 0x00020000... (20%)
...
Hash of data verified.
Hard resetting via RTS pin...
```

### After Boot
```
I (1234) main: SmartClock OS v1.0.0
I (2500) network_manager: Connected to Wi-Fi
I (3200) time_service: Time synchronized
I (4110) weather_service: Fetched: 45°F, Clear
```

### On Display
- ⏰ Current time
- 📅 Day and location ("Mon • Dry Ridge")
- 🌤️ Weather with temperature and conditions
- 🌅 Sunrise and sunset times

---

## Troubleshooting

### "Serial port not found"
- Check USB cable (must be data cable, not just charging)
- Install drivers: CH340 or CP2102
- Try a different USB port

### "Failed to connect"
1. Hold BOOT button on ESP32
2. Press RESET button
3. Release BOOT button
4. Run flash command again

### "Build failed"
- Make sure you're using "ESP-IDF 5.x CMD" not regular CMD
- Run: `idf.py fullclean` then `idf.py build`

### Exit monitor
Press: `Ctrl + ]`

---

## Common Commands

| Command | What it does |
|---------|-------------|
| `idf.py build` | Build firmware |
| `idf.py -p auto flash` | Flash to ESP32 |
| `idf.py -p auto monitor` | View serial output |
| `idf.py -p auto flash monitor` | Flash + monitor |
| `idf.py -p auto app-flash` | Quick flash (app only) |

---

## First Time Setup

### Configure Wi-Fi

**Option 1: Provisioning (Recommended)**
1. ESP32 creates Wi-Fi AP: "SmartClock-XXXX"
2. Connect with phone/computer
3. Web page opens automatically
4. Enter your Wi-Fi credentials
5. Done! Settings saved to ESP32

**Option 2: Hardcode (for testing)**
1. Edit `firmware/main/config.h`:
   ```c
   #define WIFI_SSID "YourWiFiName"
   #define WIFI_PASSWORD "YourPassword"
   ```
2. Rebuild: `idf.py build`
3. Flash: `idf.py -p auto app-flash`

---

## Your Location

Already configured for **Dry Ridge, Kentucky**:
- Latitude: 38.6820
- Longitude: -84.5894
- Timezone: Eastern Time

To change location, edit `firmware/main/config.h` and rebuild.

---

## Need Help?

📖 Full guide: `SETUP_GUIDE.md`
🔧 Technical details: `PORTING_GUIDE.md`
📝 Configuration: `firmware/main/config.h`

---

## Summary: 3 Commands to Success

1. **Install ESP-IDF:** Run `install_esp_idf.bat`
2. **Build:** `idf.py build`
3. **Flash:** `idf.py -p auto flash monitor`

You're ready to go! 🚀
