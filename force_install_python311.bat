@echo off
echo ========================================
echo Force ESP-IDF Installation with Python 3.11
echo ========================================
echo.

REM This script bypasses the default system Python and forces ESP-IDF to use Python 3.11

echo Step 1: Cleaning any existing ESP-IDF Python environment...
echo.

if exist "C:\Users\logan\.espressif\python_env" (
    echo Removing old Python environment...
    rd /s /q "C:\Users\logan\.espressif\python_env"
    echo Done.
) else (
    echo No existing environment found. Skipping cleanup.
)

echo.
echo Step 2: Setting Python path to 3.11...
echo.

set PYTHON=C:\Users\logan\AppData\Local\Programs\Python\Python311\python.exe

echo Using Python at: %PYTHON%
echo.

REM Verify Python 3.11 exists
if not exist "%PYTHON%" (
    echo ERROR: Python 3.11 not found at %PYTHON%
    echo Please verify the installation path.
    pause
    exit /b 1
)

REM Check Python version
echo Verifying Python version...
"%PYTHON%" --version
echo.

echo Step 3: Navigating to ESP-IDF directory...
echo.

cd /d "C:\Users\logan\esp-idf-v5.5.2"

if %errorLevel% neq 0 (
    echo ERROR: ESP-IDF directory not found at C:\Users\logan\esp-idf-v5.5.2
    echo Please verify the installation path.
    pause
    exit /b 1
)

echo Step 4: Running ESP-IDF installation with Python 3.11...
echo This will take 5-10 minutes...
echo.

install.bat esp32

echo.
echo ========================================
echo Installation Complete!
echo ========================================
echo.

if %errorLevel% equ 0 (
    echo SUCCESS! ESP-IDF is now installed with Python 3.11.
    echo.
    echo Next steps:
    echo 1. Close this window
    echo 2. Open "ESP-IDF 5.5 CMD" from Start Menu
    echo 3. Navigate to: cd "C:\Users\logan\Downloads\SmartClock OS Codex\SmartClockOS\firmware"
    echo 4. Run: idf.py build
    echo.
) else (
    echo.
    echo Installation failed. Check the error messages above.
    echo.
)

pause
