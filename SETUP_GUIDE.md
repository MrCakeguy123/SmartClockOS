# SmartClock OS Setup Guide

Complete guide to set up your development environment and flash the SmartClock OS to your ESP32 when it arrives.

## Prerequisites

### Hardware Requirements
- **ESP32 Development Board** (any ESP32 variant)
  - Recommended: ESP32-WROVER or ESP32-WROOM
  - USB-C or Micro-USB cable for programming
- **ST7796 Display** (480x320 resolution)
- **Capacitive Touch Controller** (I2C)

### Software Requirements
- **Operating System:** Windows 10/11 (64-bit)
- **Python:** 3.8 or newer
- **Git:** For version control
- **ESP-IDF:** v5.x (we'll install this)

---

## Step 1: Install ESP-IDF

ESP-IDF is the official development framework for ESP32. Here's how to install it:

### Option A: Using ESP-IDF Windows Installer (Recommended)

1. **Download the ESP-IDF Windows Installer:**
   - Go to: https://dl.espressif.com/dl/esp-idf/
   - Download the latest ESP-IDF v5.x installer (e.g., `esp-idf-tools-setup-5.3.exe`)

2. **Run the installer:**
   - Double-click the downloaded installer
   - Choose "Install ESP-IDF"
   - Select installation location (default: `C:\Espressif\`)
   - Install all components (Python, Git, toolchain)
   - Wait for installation to complete (10-20 minutes)

3. **Verify installation:**
   - Open "ESP-IDF 5.x CMD" from Start Menu
   - Run: `idf.py --version`
   - Should show: `ESP-IDF v5.x.x`

### Option B: Manual Installation

If the installer doesn't work, follow the manual installation guide:
https://docs.espressif.com/projects/esp-idf/en/latest/esp32/get-started/windows-setup.html

---

## Step 2: Set Up the Project

1. **Navigate to the firmware directory:**
   ```cmd
   cd "C:\Users\logan\Downloads\SmartClock OS Codex\SmartClockOS\firmware"
   ```

2. **Open ESP-IDF environment:**
   - Open "ESP-IDF 5.x CMD" from Start Menu
   - Navigate to the firmware directory

3. **Configure the project:**
   ```cmd
   idf.py menuconfig
   ```

   **Important configurations:**
   - **Component config → ESP System Settings:**
     - Main task stack size: 4096 (default is fine)
   - **Component config → FreeRTOS:**
     - Tick rate (Hz): 1000
   - **Serial flasher config:**
     - Flash size: 4 MB (or match your ESP32)
     - Flash SPI speed: 40 MHz
     - Flash SPI mode: DIO
   - **Partition Table:**
     - Partition table: Single factory app, no OTA

   Press `S` to save, then `Q` to quit.

4. **Build the firmware:**
   ```cmd
   idf.py build
   ```

   This will:
   - Compile all source files
   - Link libraries
   - Generate the firmware binary
   - Take 2-5 minutes on first build

   **Success message:**
   ```
   Project build complete. To flash, run:
   idf.py -p (PORT) flash
   ```

---

## Step 3: Configure Your Location

Before building, make sure your location is set correctly in `firmware/main/config.h`:

```c
// Your location coordinates
#define WEATHER_LAT 38.6820   // Dry Ridge, Kentucky
#define WEATHER_LON -84.5894

// Display location name
#define LOCATION_NAME "Dry Ridge"

// Timezone (Eastern Time)
#define TIMEZONE_STRING "EST5EDT,M3.2.0,M11.1.0"
```

If you need to change these, edit the file and rebuild:
```cmd
idf.py build
```

---

## Step 4: When Your ESP32 Arrives

### 4.1 Connect the ESP32

1. **Connect via USB:**
   - Plug your ESP32 into your computer via USB cable
   - Wait for Windows to install drivers automatically

2. **Find the COM port:**
   - Open Device Manager (Win+X → Device Manager)
   - Expand "Ports (COM & LPT)"
   - Look for "Silicon Labs CP210x" or "USB-SERIAL CH340"
   - Note the COM port number (e.g., COM3, COM4)

   **Alternative method:**
   ```cmd
   idf.py -p auto flash
   ```
   This will auto-detect the port.

### 4.2 Flash the Firmware

1. **Open ESP-IDF CMD:**
   - Launch "ESP-IDF 5.x CMD" from Start Menu

2. **Navigate to firmware directory:**
   ```cmd
   cd "C:\Users\logan\Downloads\SmartClock OS Codex\SmartClockOS\firmware"
   ```

3. **Flash the firmware:**
   ```cmd
   idf.py -p COM3 flash
   ```
   Replace `COM3` with your actual port number.

   **Or use auto-detection:**
   ```cmd
   idf.py -p auto flash
   ```

4. **Monitor the output:**
   ```cmd
   idf.py -p COM3 monitor
   ```

   This will show you the boot logs and debug output.

   **Exit monitor:** Press `Ctrl+]`

### 4.3 Flash + Monitor in One Command

For convenience, you can flash and monitor in a single command:

```cmd
idf.py -p COM3 flash monitor
```

---

## Step 5: First Boot

### What to Expect

1. **Boot sequence:**
   - Loading screen with "SmartClock OS"
   - Progress bar from 0-100%
   - Shows loading status for each component

2. **Wi-Fi provisioning:**
   - On first boot, ESP32 creates a Wi-Fi access point
   - Status shows "Connecting to Wi-Fi"
   - Connect to the AP with your phone/computer
   - Configure your Wi-Fi credentials via web portal

3. **After Wi-Fi connection:**
   - Clock face appears
   - Time synchronizes via NTP
   - Weather data loads from Open-Meteo API
   - Display shows:
     - Current time (24-hour format)
     - Day of week and location ("Mon • Dry Ridge")
     - Weather card with temperature, conditions, high/low
     - Sunrise and sunset times

### Serial Monitor Output

You should see logs like:
```
I (1234) main: SmartClock OS v1.0.0
I (1250) network_manager: Starting Wi-Fi
I (2500) network_manager: Connected to Wi-Fi
I (2510) time_service: Syncing time via NTP
I (3200) time_service: Time synchronized
I (3210) weather_service: Fetching weather from: https://api.open-meteo.com/...
I (4100) weather_service: HTTP Status = 200, content_length = 1234
I (4110) weather_service: Fetched: 45°F (feels 42°F), Hi:48° Lo:38°, Clear
I (4120) weather_service: Sunrise: 7:15 AM, Sunset: 5:42 PM
```

---

## Step 6: Troubleshooting

### Build Errors

**Error: "CMake Error: Could not find ESP-IDF"**
- Solution: Make sure you're using ESP-IDF CMD, not regular CMD
- Or set IDF_PATH environment variable

**Error: "esp_http_client.h: No such file or directory"**
- Solution: Update ESP-IDF to v5.x or newer
- Run: `git submodule update --init --recursive`

**Error: "Python not found"**
- Solution: Install Python 3.8+ from python.org
- Add to PATH during installation

### Flash Errors

**Error: "Serial port not found"**
- Check USB cable (must support data, not just charging)
- Install CH340/CP2102 drivers manually
- Try different USB port

**Error: "Failed to connect to ESP32"**
- Hold BOOT button while connecting
- Press RESET button before flashing
- Lower baud rate: `idf.py -b 115200 flash`

**Error: "Timed out waiting for packet header"**
- Press and hold BOOT button
- Click RESET button
- Release BOOT button
- Try flashing again

### Runtime Errors

**Weather shows "Offline"**
- Check Wi-Fi credentials
- Verify internet connection
- Check monitor output for HTTP errors
- Ensure firewall allows ESP32 internet access

**Display not working**
- Check display connections
- Verify SPI pins in menuconfig
- Check display model (ST7796 required)

**Touch not responding**
- Verify touch controller I2C address
- Check I2C pins configuration
- Update touch driver if needed

**Time not syncing**
- Check NTP server (pool.ntp.org)
- Verify timezone string in config.h
- Ensure Wi-Fi has internet access

---

## Step 7: Development Workflow

### Making Changes

1. **Edit code files** (e.g., config.h, ui_shell.c)

2. **Rebuild:**
   ```cmd
   idf.py build
   ```

3. **Flash:**
   ```cmd
   idf.py -p COM3 flash
   ```

4. **Monitor:**
   ```cmd
   idf.py -p COM3 monitor
   ```

### Quick Commands

| Command | Description |
|---------|-------------|
| `idf.py menuconfig` | Open configuration menu |
| `idf.py build` | Build the project |
| `idf.py clean` | Clean build files |
| `idf.py fullclean` | Full clean (including config) |
| `idf.py -p COM3 flash` | Flash firmware to ESP32 |
| `idf.py -p COM3 monitor` | Monitor serial output |
| `idf.py -p COM3 flash monitor` | Flash and monitor |
| `idf.py -p COM3 erase-flash` | Erase entire flash |
| `idf.py app-flash` | Flash only app (faster) |

### Updating Location

1. Edit `firmware/main/config.h`:
   ```c
   #define WEATHER_LAT 38.6820
   #define WEATHER_LON -84.5894
   #define LOCATION_NAME "Dry Ridge"
   ```

2. Rebuild and flash:
   ```cmd
   idf.py build
   idf.py -p COM3 app-flash
   ```

---

## Step 8: Advanced Configuration

### Changing Display Settings

Edit `menuconfig` for display configuration:
```cmd
idf.py menuconfig
```

Navigate to **Component config → LVGL configuration:**
- Display resolution: 480x320
- Color depth: 16-bit RGB565
- Buffer size: 1/10 screen size

### Changing Power Settings

Edit `firmware/main/config.h`:
```c
#define AUTO_DIM_TIMEOUT_SEC 30      // Dim after 30 seconds
#define AUTO_BLANK_TIMEOUT_SEC 60    // Blank after 60 seconds
#define DEEP_SLEEP_TIMEOUT_SEC 600   // Sleep after 10 minutes
#define NIGHT_MODE_START_HOUR 22     // 10 PM
#define NIGHT_MODE_END_HOUR 6        // 6 AM
```

### Changing Weather Update Interval

Edit `firmware/main/ui_shell.c` line 115:
```c
if (ctx->weather_ticks >= 300 && ctx->config.weather_request_cb) {
    // 300 seconds = 5 minutes
    // Change to 600 for 10 minutes, etc.
```

---

## Step 9: WiFi Configuration

### Option 1: Hardcode Wi-Fi (for testing)

Edit `firmware/main/config.h`:
```c
#define WIFI_SSID "YourNetworkName"
#define WIFI_PASSWORD "YourPassword"
```

### Option 2: Provisioning Portal (recommended)

1. Leave WIFI_SSID and WIFI_PASSWORD empty in config.h
2. On first boot, ESP32 creates AP: "SmartClock-XXXX"
3. Connect to this AP with phone/computer
4. Browser opens to configuration portal
5. Enter your Wi-Fi credentials
6. ESP32 connects to your network
7. Credentials saved in NVS flash

---

## Useful Links

- **ESP-IDF Documentation:** https://docs.espressif.com/projects/esp-idf/en/latest/
- **LVGL Documentation:** https://docs.lvgl.io/
- **Open-Meteo API:** https://open-meteo.com/
- **ESP32 Pinout Reference:** https://randomnerdtutorials.com/esp32-pinout-reference-gpios/
- **Timezone Database:** https://github.com/nayarsystems/posix_tz_db/blob/master/zones.csv

---

## Pre-Build Checklist

Before your ESP32 arrives, make sure you have:

- [x] ESP-IDF v5.x installed
- [x] Firmware builds successfully (`idf.py build`)
- [x] Location configured in config.h
- [x] Timezone set correctly
- [x] Wi-Fi credentials set (or ready for provisioning)
- [x] USB cable ready (data-capable)
- [x] Serial drivers installed (if needed)

---

## Quick Start Summary

When your ESP32 arrives, just run these 3 commands:

```cmd
cd "C:\Users\logan\Downloads\SmartClock OS Codex\SmartClockOS\firmware"
idf.py -p auto flash monitor
```

That's it! Your SmartClock OS will be running.

---

## Support

If you encounter any issues:

1. Check the troubleshooting section above
2. Review serial monitor output for error messages
3. Check ESP-IDF documentation
4. Verify all connections and hardware

Happy building! 🚀
