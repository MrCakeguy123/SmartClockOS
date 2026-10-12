#pragma once

#include <Arduino.h>
#include <Preferences.h>
#include <time.h>

#define CLOCK_VERSION "2.8.0"
#define CLOCK_NAME "SmartClockOS"

#define SCREEN_WIDTH 800
#define SCREEN_HEIGHT 480

#define RGB_DE 41
#define RGB_VSYNC 40
#define RGB_HSYNC 39
#define RGB_PCLK 42
#define RGB_BL 2

#define RGB_R0 14
#define RGB_R1 21
#define RGB_R2 47
#define RGB_R3 48
#define RGB_R4 45

#define RGB_G0 9
#define RGB_G1 46
#define RGB_G2 3
#define RGB_G3 8
#define RGB_G4 16
#define RGB_G5 1

#define RGB_B0 15
#define RGB_B1 7
#define RGB_B2 6
#define RGB_B3 5
#define RGB_B4 4

#define RGB_H_FRONT 210
#define RGB_H_PULSE 30
#define RGB_H_BACK 16
#define RGB_V_FRONT 22
#define RGB_V_PULSE 13
#define RGB_V_BACK 10
#define RGB_PCLK_POL 1
#define RGB_PCLK_FREQ 14000000

#define TOUCH_SDA 19
#define TOUCH_SCL 20
#define TOUCH_INT 18
#define TOUCH_RST 38
#define TOUCH_ADDR 0x5D
#define TOUCH_ADDR_ALT 0x14

#define BME280_SDA 11
#define BME280_SCL 12
#define BME280_ADDR 0x76
#define BME280_INTERVAL_MS 10000

#define BACKLIGHT_PWM_FREQ 5000
#define BACKLIGHT_PWM_BITS 8
#define BACKLIGHT_MAX ((1 << BACKLIGHT_PWM_BITS) - 1)

#define STATUS_BAR_HEIGHT 32
#define KEYBOARD_HEIGHT 200
#define UI_FRAME_MS 33

#define WIFI_CONNECT_TIMEOUT_MS 15000
#define WIFI_CHECK_INTERVAL_MS 5000

#define WEATHER_INTERVAL_MS 600000
// The UI only ever renders 24 hourly cards, but this array is instantiated
// THREE times (DataManager, Weather, UI) in internal RAM. At 168 that was
// ~35 KB of DRAM held hostage for data nothing displays — starving the
// mbedTLS handshake and breaking every weather fetch after boot.
#define WEATHER_MAX_HOURLY 24
#define WEATHER_FORECAST_HOURS 24
#define WEATHER_MAX_DAILY 7

#define NTP_SERVER "pool.ntp.org"
#define TIMEZONE "EST5EDT,M3.2.0,M11.1.0"

#define ENABLE_BME280 1
#define ENABLE_ANIMATIONS 1
#define ENABLE_SPLASH 1
#define ENABLE_FAN 1

#if ENABLE_FAN
// 30mm 5V fan, low-side MOSFET on IO17 (exposed on P3/P5 JST)
#define FAN_PIN 17
#define FAN_PWM_FREQ 25000
#define FAN_PWM_BITS 8
#define FAN_ON_C 55.0f            // spin up above this die temp
#define FAN_OFF_C 48.0f           // stop below this (hysteresis)
#define FAN_MAX_C 70.0f           // 100% duty at/above this
#define FAN_MIN_DUTY 30           // % floor so the fan reliably starts
#define FAN_NIGHT_OVERRIDE_C 75.0f // night mode stays silent below this
#endif

enum TempUnit { TEMP_F = 0, TEMP_C = 1 };
enum WindUnit { WIND_MPH = 0, WIND_KMH = 1 };
enum PrecipUnit { PRECIP_IN = 0, PRECIP_MM = 1 };

struct Settings {
  uint8_t version = 2;
  uint8_t brightness = BACKLIGHT_MAX;
  TempUnit tempUnit = TEMP_F;
  WindUnit windUnit = WIND_MPH;
  PrecipUnit precipUnit = PRECIP_IN;
  char wifiSsid[33] = {0};
  char wifiPass[65] = {0};
  // Night mode settings
  uint8_t nightBrightness = 30;
  uint8_t nightStartHour = 22; // 10 PM
  uint8_t nightEndHour = 6;    // 6 AM
  bool nightModeEnabled = false;
};

inline const char *tempSuffix(TempUnit u) { return u == TEMP_C ? "C" : "F"; }
inline const char *windSuffix(WindUnit u) {
  return u == WIND_KMH ? "km/h" : "mph";
}
inline const char *precipSuffix(PrecipUnit u) {
  return u == PRECIP_MM ? "mm" : "in";
}
inline const char *tempApiUnit(TempUnit u) {
  return u == TEMP_C ? "celsius" : "fahrenheit";
}
inline const char *windApiUnit(WindUnit u) {
  return u == WIND_KMH ? "kmh" : "mph";
}
inline const char *precipApiUnit(PrecipUnit u) {
  return u == PRECIP_MM ? "mm" : "inch";
}

// True when night mode should currently dim the display. Treats "clock not
// yet NTP-synced" as night so a reboot at 3 AM doesn't blast full brightness.
inline bool isNightModeActiveNow(const Settings &s) {
  if (!s.nightModeEnabled)
    return false;

  int startHour = constrain((int)s.nightStartHour, 0, 23);
  int endHour = constrain((int)s.nightEndHour, 0, 23);
  if (startHour == endHour)
    return true;

  time_t now = time(nullptr);
  if (now < 1700000000)
    return true;

  struct tm ti;
  localtime_r(&now, &ti);
  int hour = ti.tm_hour;

  if (startHour < endHour)
    return hour >= startHour && hour < endHour;
  return hour >= startHour || hour < endHour;
}
