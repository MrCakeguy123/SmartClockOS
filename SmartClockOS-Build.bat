@echo off
echo Activating ESP-IDF...
call C:\Users\logan\esp-idf-v5.5.2\export.bat
cd /d "C:\Users\logan\Downloads\SmartClock OS Codex\SmartClockOS"
echo.
echo ========================================
echo ESP-IDF Ready! You can now:
echo   idf.py build          - Build firmware
echo   idf.py flash          - Flash to ESP32
echo   idf.py monitor        - View serial output
echo ========================================
echo.
cmd /k
