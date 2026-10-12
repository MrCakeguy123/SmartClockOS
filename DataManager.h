#pragma once

#include "Config.h"
#include <freertos/FreeRTOS.h>
#include <freertos/portmacro.h>
#include <string.h>

struct CurrentWeather {
  int32_t observedAt = 0;
  float temp = 0;
  float humidity = 0;
  float precip = 0;
  float wind = 0;
  float windDir = 0;
  int isDay = -1;
  int weatherCode = -1;
  bool valid = false;
  char tempStr[16] = "--";
  char humidityStr[16] = "H: --";
  char windStr[16] = "W: --";
  char precipStr[16] = "P: --";
  char conditionStr[24] = "Loading...";
  char updatedStr[20] = "--";
};

struct HourlyData {
  int32_t time = 0;
  float temp = 0;
  float precipProb = 0;
  float precip = 0;
  float humidity = 0;
  float wind = 0;
  int weatherCode = -1;
  char timeStr[12] = "--";
  char tempStr[8] = "--";
  char precipStr[8] = "--";
  char conditionStr[16] = "--";
};

struct DailyData {
  int32_t time = 0;
  int32_t sunrise = 0;
  int32_t sunset = 0;
  float tempMax = 0;
  float tempMin = 0;
  float uvIndex = 0;
  float precipSum = 0;
  int weatherCode = -1;
  char dayStr[16] = "--";
  char tempRangeStr[20] = "-- / --";
  char precipStr[12] = "--";
  char conditionStr[16] = "--";
  char sunriseStr[12] = "--";
  char sunsetStr[12] = "--";
};

#if ENABLE_BME280
struct IndoorData {
  float temp = 0;
  float humidity = 0;
  float pressure = 0;
  bool valid = false;
  char tempStr[16] = "--";
  char humidityStr[12] = "--";
  uint32_t version = 0;
};
#endif

struct SystemInfo {
  char cpuStr[48] = "CPU: --";
  char ramStr[64] = "RAM: --";
  uint32_t version = 0;
};

struct NetworkState {
  bool connected = false;
  int rssi = 0;
  char timeStr[16] = "--:--";
  char timeShort[8] = "--:--";
  char ampm[4] = "";
  char dateStr[24] = "--";
};

struct DiagnosticsData {
  float fps = 0.0f;
  uint32_t heapFree = 0;
  uint32_t heapTotal = 0;
  uint32_t psramFree = 0;
  uint32_t psramTotal = 0;
  bool wifiConnected = false;
  int wifiRssi = 0;
  uint8_t wifiQuality = 0;
  uint32_t weatherLatencyMs = 0;
  bool weatherOk = false;
  bool sensorHealthy = false;
  uint32_t sensorLastMs = 0;
  float dieTempC = 0;
  uint8_t fanDutyPct = 0;
  uint32_t version = 0;
};

class DataManager {
public:
  void setTime(const char *time, const char *date,
               const char *shortTime = nullptr, const char *ampmText = nullptr) {
    portENTER_CRITICAL(&_lock);
    strncpy(_net.timeStr, time, sizeof(_net.timeStr) - 1);
    strncpy(_net.dateStr, date, sizeof(_net.dateStr) - 1);
    if (shortTime)
      strncpy(_net.timeShort, shortTime, sizeof(_net.timeShort) - 1);
    if (ampmText)
      strncpy(_net.ampm, ampmText, sizeof(_net.ampm) - 1);
    portEXIT_CRITICAL(&_lock);
  }

  NetworkState getNetwork() {
    portENTER_CRITICAL(&_lock);
    NetworkState c = _net;
    portEXIT_CRITICAL(&_lock);
    return c;
  }

  void setWifi(bool connected, int rssi = 0) {
    portENTER_CRITICAL(&_lock);
    _net.connected = connected;
    _net.rssi = connected ? rssi : 0;
    portEXIT_CRITICAL(&_lock);
  }

  void setSettings(const Settings &s) {
    portENTER_CRITICAL(&_lock);
    _settings = s;
    _settingsVer++;
    portEXIT_CRITICAL(&_lock);
  }

  Settings getSettings() {
    portENTER_CRITICAL(&_lock);
    Settings c = _settings;
    portEXIT_CRITICAL(&_lock);
    return c;
  }

  uint32_t getSettingsVersion() {
    portENTER_CRITICAL(&_lock);
    uint32_t v = _settingsVer;
    portEXIT_CRITICAL(&_lock);
    return v;
  }

  void setWeather(const CurrentWeather &cur, const HourlyData *hourly,
                  size_t hourlyCount, const DailyData *daily,
                  size_t dailyCount) {
    portENTER_CRITICAL(&_lock);
    _current = cur;
    _current.valid = true;
    _hourlyCount =
        hourlyCount > WEATHER_MAX_HOURLY ? WEATHER_MAX_HOURLY : hourlyCount;
    _dailyCount =
        dailyCount > WEATHER_MAX_DAILY ? WEATHER_MAX_DAILY : dailyCount;
    if (hourly && _hourlyCount)
      memcpy(_hourly, hourly, sizeof(HourlyData) * _hourlyCount);
    if (daily && _dailyCount)
      memcpy(_daily, daily, sizeof(DailyData) * _dailyCount);
    _weatherVer++;
    portEXIT_CRITICAL(&_lock);
  }

  CurrentWeather getCurrent() {
    portENTER_CRITICAL(&_lock);
    CurrentWeather c = _current;
    portEXIT_CRITICAL(&_lock);
    return c;
  }

  size_t getHourly(HourlyData *dest, size_t max) {
    portENTER_CRITICAL(&_lock);
    size_t n = _hourlyCount > max ? max : _hourlyCount;
    if (dest && n)
      memcpy(dest, _hourly, sizeof(HourlyData) * n);
    portEXIT_CRITICAL(&_lock);
    return n;
  }

  size_t getDaily(DailyData *dest, size_t max) {
    portENTER_CRITICAL(&_lock);
    size_t n = _dailyCount > max ? max : _dailyCount;
    if (dest && n)
      memcpy(dest, _daily, sizeof(DailyData) * n);
    portEXIT_CRITICAL(&_lock);
    return n;
  }

  uint32_t getWeatherVersion() {
    portENTER_CRITICAL(&_lock);
    uint32_t v = _weatherVer;
    portEXIT_CRITICAL(&_lock);
    return v;
  }

  void setDiagFps(float fps) {
    portENTER_CRITICAL(&_lock);
    _diag.fps = fps;
    _diag.version++;
    portEXIT_CRITICAL(&_lock);
  }

  void setDiagMemory(uint32_t heapFree, uint32_t heapTotal, uint32_t psramFree,
                     uint32_t psramTotal) {
    portENTER_CRITICAL(&_lock);
    _diag.heapFree = heapFree;
    _diag.heapTotal = heapTotal;
    _diag.psramFree = psramFree;
    _diag.psramTotal = psramTotal;
    _diag.version++;
    portEXIT_CRITICAL(&_lock);
  }

  void setDiagWifi(bool connected, int rssi, uint8_t quality) {
    portENTER_CRITICAL(&_lock);
    _diag.wifiConnected = connected;
    _diag.wifiRssi = connected ? rssi : 0;
    _diag.wifiQuality = connected ? quality : 0;
    _diag.version++;
    portEXIT_CRITICAL(&_lock);
  }

  void setDiagWeatherLatency(uint32_t ms, bool ok) {
    portENTER_CRITICAL(&_lock);
    _diag.weatherLatencyMs = ms;
    _diag.weatherOk = ok;
    _diag.version++;
    portEXIT_CRITICAL(&_lock);
  }

  void setDiagFan(float dieTemp, uint8_t dutyPct) {
    portENTER_CRITICAL(&_lock);
    _diag.dieTempC = dieTemp;
    _diag.fanDutyPct = dutyPct;
    _diag.version++;
    portEXIT_CRITICAL(&_lock);
  }

  void setDiagSensorHealth(bool healthy, uint32_t lastMs) {
    portENTER_CRITICAL(&_lock);
    _diag.sensorHealthy = healthy;
    _diag.sensorLastMs = lastMs;
    _diag.version++;
    portEXIT_CRITICAL(&_lock);
  }

  DiagnosticsData getDiagnostics() {
    portENTER_CRITICAL(&_lock);
    DiagnosticsData d = _diag;
    portEXIT_CRITICAL(&_lock);
    return d;
  }

  uint32_t getDiagnosticsVersion() {
    portENTER_CRITICAL(&_lock);
    uint32_t v = _diag.version;
    portEXIT_CRITICAL(&_lock);
    return v;
  }

  void requestWifiReconnect() {
    portENTER_CRITICAL(&_lock);
    _wifiReconnect = true;
    portEXIT_CRITICAL(&_lock);
  }

  bool consumeWifiReconnect() {
    portENTER_CRITICAL(&_lock);
    bool r = _wifiReconnect;
    _wifiReconnect = false;
    portEXIT_CRITICAL(&_lock);
    return r;
  }

  void requestWeatherRefresh() {
    portENTER_CRITICAL(&_lock);
    _weatherRefresh = true;
    portEXIT_CRITICAL(&_lock);
  }

  bool consumeWeatherRefresh() {
    portENTER_CRITICAL(&_lock);
    bool r = _weatherRefresh;
    _weatherRefresh = false;
    portEXIT_CRITICAL(&_lock);
    return r;
  }

#if ENABLE_BME280
  void setIndoor(const IndoorData &d) {
    portENTER_CRITICAL(&_lock);
    // Preserve the running version counter across the struct copy, otherwise
    // every update lands on version 1 and the UI never sees a change again.
    uint32_t ver = _indoor.version;
    _indoor = d;
    _indoor.version = ver + 1;
    portEXIT_CRITICAL(&_lock);
  }

  IndoorData getIndoor() {
    portENTER_CRITICAL(&_lock);
    IndoorData c = _indoor;
    portEXIT_CRITICAL(&_lock);
    return c;
  }
#endif

  void setSystemInfo(const char *cpu, const char *ram) {
    portENTER_CRITICAL(&_lock);
    strncpy(_sys.cpuStr, cpu, sizeof(_sys.cpuStr) - 1);
    strncpy(_sys.ramStr, ram, sizeof(_sys.ramStr) - 1);
    _sys.version++;
    portEXIT_CRITICAL(&_lock);
  }

  SystemInfo getSystemInfo() {
    portENTER_CRITICAL(&_lock);
    SystemInfo c = _sys;
    portEXIT_CRITICAL(&_lock);
    return c;
  }

private:
  portMUX_TYPE _lock = portMUX_INITIALIZER_UNLOCKED;
  NetworkState _net;
  Settings _settings;
  uint32_t _settingsVer = 0;
  CurrentWeather _current;
  HourlyData _hourly[WEATHER_MAX_HOURLY];
  DailyData _daily[WEATHER_MAX_DAILY];
  size_t _hourlyCount = 0;
  size_t _dailyCount = 0;
  uint32_t _weatherVer = 0;
  bool _wifiReconnect = false;
  bool _weatherRefresh = false;
#if ENABLE_BME280
  IndoorData _indoor;
#endif
  SystemInfo _sys;
  DiagnosticsData _diag;
};

extern DataManager Data;

class SettingsManager {
public:
  void begin() { _prefs.begin("smartclock", false); }

  Settings load() {
    Settings s;
    uint8_t savedVer = _prefs.getUChar("ver", 0);
    if (savedVer == 0)
      return s; // First run

    s.brightness = _prefs.getUChar("bl", s.brightness);
    s.tempUnit = (TempUnit)_prefs.getUChar("tu", s.tempUnit);
    s.windUnit = (WindUnit)_prefs.getUChar("wu", s.windUnit);
    s.precipUnit = (PrecipUnit)_prefs.getUChar("pu", s.precipUnit);

    String ssid = _prefs.getString("ssid", "");
    String pass = _prefs.getString("pass", "");
    strncpy(s.wifiSsid, ssid.c_str(), sizeof(s.wifiSsid) - 1);
    strncpy(s.wifiPass, pass.c_str(), sizeof(s.wifiPass) - 1);

    // Load night mode fields (defaults if not present)
    if (savedVer >= 2) {
      s.nightBrightness = _prefs.getUChar("nb", s.nightBrightness);
      s.nightStartHour = _prefs.getUChar("nsh", s.nightStartHour);
      s.nightEndHour = _prefs.getUChar("neh", s.nightEndHour);
      s.nightModeEnabled = _prefs.getBool("nme", s.nightModeEnabled);
    }

    if (s.brightness > BACKLIGHT_MAX)
      s.brightness = BACKLIGHT_MAX;
    return s;
  }

  void save(const Settings &s) {
    _prefs.putUChar("ver", s.version);
    _prefs.putUChar("bl", s.brightness);
    _prefs.putUChar("tu", (uint8_t)s.tempUnit);
    _prefs.putUChar("wu", (uint8_t)s.windUnit);
    _prefs.putUChar("pu", (uint8_t)s.precipUnit);
    _prefs.putString("ssid", s.wifiSsid);
    _prefs.putString("pass", s.wifiPass);
    // Night mode fields
    _prefs.putUChar("nb", s.nightBrightness);
    _prefs.putUChar("nsh", s.nightStartHour);
    _prefs.putUChar("neh", s.nightEndHour);
    _prefs.putBool("nme", s.nightModeEnabled);
  }

private:
  Preferences _prefs;
};

extern SettingsManager SettingsStore;
