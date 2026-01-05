@echo off
echo ========================================
echo SmartClock OS - ESP-IDF Setup Script
echo ========================================
echo.

REM Check if running as administrator
net session >nul 2>&1
if %errorLevel% neq 0 (
    echo WARNING: Not running as administrator.
    echo Some operations may fail. Consider running as admin.
    echo.
    pause
)

echo This script will help you install ESP-IDF v5.x
echo.
echo Choose installation method:
echo 1. Download and run ESP-IDF Windows Installer (RECOMMENDED)
echo 2. Manual installation instructions
echo 3. Skip installation (ESP-IDF already installed)
echo.

set /p choice="Enter your choice (1-3): "

if "%choice%"=="1" goto installer
if "%choice%"=="2" goto manual
if "%choice%"=="3" goto verify
goto invalid

:installer
echo.
echo Opening ESP-IDF download page...
echo.
echo Please download: esp-idf-tools-setup-5.x.exe
echo Installation location: C:\Espressif\
echo.
start https://dl.espressif.com/dl/esp-idf/
echo.
echo After installation:
echo 1. Open "ESP-IDF 5.x CMD" from Start Menu
echo 2. Navigate to: C:\Users\logan\Downloads\SmartClock OS Codex\SmartClockOS\firmware
echo 3. Run: idf.py build
echo.
pause
goto end

:manual
echo.
echo Manual Installation Steps:
echo.
echo 1. Install Python 3.8 or newer from python.org
echo 2. Install Git from git-scm.com
echo 3. Open Command Prompt and run:
echo    mkdir C:\Espressif
echo    cd C:\Espressif
echo    git clone --recursive https://github.com/espressif/esp-idf.git
echo    cd esp-idf
echo    install.bat esp32
echo    export.bat
echo.
echo 4. Add ESP-IDF to PATH or use ESP-IDF CMD shortcut
echo.
echo Full guide: https://docs.espressif.com/projects/esp-idf/en/latest/esp32/get-started/windows-setup.html
echo.
pause
goto end

:verify
echo.
echo Verifying ESP-IDF installation...
echo.

REM Try to find idf.py
where idf.py >nul 2>&1
if %errorLevel% equ 0 (
    echo [OK] ESP-IDF found in PATH
    idf.py --version
    echo.
    echo ESP-IDF is ready to use!
    goto buildtest
) else (
    echo [!] ESP-IDF not found in PATH
    echo.
    echo Please use "ESP-IDF 5.x CMD" from Start Menu
    echo or run this from ESP-IDF environment.
    echo.
    pause
    goto end
)

:buildtest
echo.
echo Would you like to test build the firmware now? (y/n)
set /p build="Enter choice: "
if /i "%build%"=="y" goto dobuild
goto end

:dobuild
echo.
echo Navigating to firmware directory...
cd /d "C:\Users\logan\Downloads\SmartClock OS Codex\SmartClockOS\firmware"

echo.
echo Building firmware...
echo This will take 2-5 minutes on first build...
echo.

idf.py build

if %errorLevel% equ 0 (
    echo.
    echo ========================================
    echo BUILD SUCCESSFUL!
    echo ========================================
    echo.
    echo Firmware is ready to flash!
    echo When your ESP32 arrives, run:
    echo   idf.py -p auto flash monitor
    echo.
) else (
    echo.
    echo ========================================
    echo BUILD FAILED
    echo ========================================
    echo.
    echo Check the error messages above.
    echo Common fixes:
    echo - Use ESP-IDF CMD instead of regular CMD
    echo - Update ESP-IDF: git pull; git submodule update
    echo - Clean build: idf.py fullclean then idf.py build
    echo.
)

pause
goto end

:invalid
echo Invalid choice. Please run the script again.
pause
goto end

:end
echo.
echo See SETUP_GUIDE.md for detailed instructions.
echo.
pause
