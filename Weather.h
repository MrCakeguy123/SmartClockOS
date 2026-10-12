#pragma once

#include "Config.h"
#include "DataManager.h"
#include "Secrets.h"
#include "WebConfig.h"
#include <Arduino.h>
#include <ArduinoJson.h>
#include <HTTPClient.h>
#include <esp_heap_caps.h>
#include <WiFiClient.h>
#include <WiFiClientSecure.h>
#include <time.h>

namespace Weather {

// Park the parsed JSON in PSRAM (8 MB, mostly idle) instead of internal
// DRAM, which is the scarce resource the TLS handshake competes for.
struct PsramAllocator : ArduinoJson::Allocator {
  void *allocate(size_t n) override {
    void *p = heap_caps_malloc(n, MALLOC_CAP_SPIRAM | MALLOC_CAP_8BIT);
    return p ? p : malloc(n); // fall back to DRAM if PSRAM is gone
  }
  void deallocate(void *p) override { heap_caps_free(p); }
  void *reallocate(void *p, size_t n) override {
    void *q = heap_caps_realloc(p, n, MALLOC_CAP_SPIRAM | MALLOC_CAP_8BIT);
    return q ? q : realloc(p, n);
  }
};

static void formatTime(int32_t epoch, const char *fmt, char *buf, size_t len) {
  if (epoch <= 0) {
    snprintf(buf, len, "--");
    return;
  }
  time_t t = (time_t)epoch;
  struct tm ti;
  localtime_r(&t, &ti);
  strftime(buf, len, fmt, &ti);
}

// WMO Weather Interpretation Codes (WW) → display string
static const char *wmoToCondition(int code, bool isDay,
                                  bool shortLabel = false) {
  switch (code) {
  case 0:
    return isDay ? (shortLabel ? "Sunny" : "Clear") : "Clear Night";
  case 1:
    return isDay ? (shortLabel ? "Sunny" : "Clear") : "Clear Night";
  case 2:
    return shortLabel ? "P.Cloudy" : "Partly Cloudy";
  case 3:
    return "Cloudy";
  case 45:
  case 48:
    return shortLabel ? "Fog" : "Foggy";
  case 51:
  case 53:
  case 55:
    return shortLabel ? "Rain" : "Rainy"; // drizzle
  case 56:
  case 57:
    return shortLabel ? "Rain" : "Rainy"; // freezing drizzle
  case 61:
  case 63:
  case 65:
    return shortLabel ? "Rain" : "Rainy";
  case 66:
  case 67:
    return shortLabel ? "Rain" : "Rainy"; // freezing rain
  case 71:
  case 73:
  case 75:
    return shortLabel ? "Snow" : "Snowy";
  case 77:
    return shortLabel ? "Snow" : "Snowy"; // snow grains
  case 80:
  case 81:
  case 82:
    return shortLabel ? "Rain" : "Rainy"; // rain showers
  case 85:
  case 86:
    return shortLabel ? "Snow" : "Snowy"; // snow showers
  case 95:
    return shortLabel ? "Thunder" : "Thunderstorm";
  case 96:
  case 99:
    return shortLabel ? "Thunder" : "Thunderstorm"; // thunderstorm + hail
  default:
    return isDay ? (shortLabel ? "Sunny" : "Clear") : "Clear Night";
  }
}

// Fallback: guess condition from precip/humidity when WMO code unavailable
static const char *guessCondition(float precip, float humidity, bool isDay,
                                  bool shortLabel = false) {
  if (precip > 5.0f)
    return shortLabel ? "Thunder" : "Thunderstorm";
  if (precip > 0.5f)
    return shortLabel ? "Rain" : "Rainy";
  if (precip > 0.1f)
    return shortLabel ? "P.Cloudy" : "Partly Cloudy";
  if (humidity > 95.0f)
    return shortLabel ? "Fog" : "Foggy";
  if (humidity > 80.0f)
    return "Cloudy";
  return isDay ? (shortLabel ? "Sunny" : "Clear") : "Clear Night";
}

static const char *getCondition(int weatherCode, float precip, float humidity,
                                bool isDay) {
  if (weatherCode >= 0)
    return wmoToCondition(weatherCode, isDay);
  return guessCondition(precip, humidity, isDay);
}

static const char *getHourlyCondition(const HourlyData &h, bool isDay) {
  if (h.weatherCode >= 0)
    return wmoToCondition(h.weatherCode, isDay, true);
  return guessCondition(h.precip, h.humidity, isDay, true);
}

static const char *getDailyCondition(const DailyData &d) {
  if (d.weatherCode >= 0)
    return wmoToCondition(d.weatherCode, true, true);
  return guessCondition(d.precipSum, 0, true, true);
}

void fetch() {
  if (WiFi.status() != WL_CONNECTED)
    return;

  Settings s = Data.getSettings();

  String url = String("https://api.open-meteo.com/v1/forecast") +
               "?latitude=" + String(WEATHER_LAT, 4) +
               "&longitude=" + String(WEATHER_LON, 4) +
               "&current=temperature_2m,relative_humidity_2m,is_day,"
               "precipitation,wind_speed_10m,wind_direction_10m,weather_code" +
               "&hourly=temperature_2m,precipitation_probability,precipitation,"
               "relative_humidity_2m,wind_speed_10m,weather_code" +
               "&daily=sunrise,sunset,temperature_2m_max,temperature_2m_min,uv_"
               "index_max,precipitation_sum,weather_code" +
               "&timezone=auto&timeformat=unixtime" +
               "&forecast_hours=" + String(WEATHER_FORECAST_HOURS) +
               "&wind_speed_unit=" + String(windApiUnit(s.windUnit)) +
               "&temperature_unit=" + String(tempApiUnit(s.tempUnit)) +
               "&precipitation_unit=" + String(precipApiUnit(s.precipUnit));

  static PsramAllocator jsonAlloc;
  JsonDocument doc(&jsonAlloc);

  int code = -1;
  uint32_t latency = 0;
  bool parsed = false;

  for (int attempt = 0; attempt < 2 && !parsed; attempt++) {
    WiFiClientSecure client;
    client.setInsecure();
    HTTPClient http;
    http.setConnectTimeout(8000);
    http.setTimeout(12000);
    http.setReuse(false);
    http.begin(client, url);
    http.addHeader("User-Agent",
                   String(CLOCK_NAME) + "/" + CLOCK_VERSION + " (weather)");
    http.addHeader("Accept", "application/json");

    uint32_t t0 = millis();
    code = http.GET();
    latency = millis() - t0;

    if (code == 200) {
      // NOTE: must go through getString(), NOT getStream(). Open-Meteo replies
      // with Transfer-Encoding: chunked and getStream() exposes the raw chunk
      // framing, which makes ArduinoJson fail with InvalidInput. getString()
      // de-chunks. The body is only ~2.5 KB now, so this costs nothing.
      String body = http.getString();
      http.end();
      DeserializationError err = deserializeJson(doc, body);
      if (!err) {
        parsed = true;
        Data.setDiagWeatherLatency(latency, true);
        break;
      }
      WebConfig::logPrintf("[Weather] JSON error (try %d/2): %s\n", attempt + 1,
                           err.c_str());
    } else {
      // These failures used to go only to Serial, which is unreachable on a
      // wall-mounted clock. Heap numbers included: a fast negative code with
      // a small largest-free-block means the TLS handshake was starved.
      WebConfig::logPrintf(
          "[Weather] HTTP %d (try %d/2) after %lums | DRAM free %u, largest %u\n",
          code, attempt + 1, (unsigned long)latency,
          (unsigned)heap_caps_get_free_size(MALLOC_CAP_INTERNAL),
          (unsigned)heap_caps_get_largest_free_block(MALLOC_CAP_INTERNAL));
      http.end();
    }

    if (attempt == 0)
      delay(400);
  }

  if (!parsed) {
    Data.setDiagWeatherLatency(latency, false);
    return;
  }

  CurrentWeather cur;
  JsonObject c = doc["current"];
  cur.observedAt = c["time"] | 0;
  cur.temp = c["temperature_2m"] | 0.0f;
  cur.humidity = c["relative_humidity_2m"] | 0.0f;
  cur.isDay = c["is_day"] | -1;
  cur.precip = c["precipitation"] | 0.0f;
  cur.wind = c["wind_speed_10m"] | 0.0f;
  cur.windDir = c["wind_direction_10m"] | 0.0f;
  cur.weatherCode = c["weather_code"] | -1;
  cur.valid = true;
  if (cur.observedAt > 0) {
    time_t obs = (time_t)cur.observedAt;
    struct tm ti;
    localtime_r(&obs, &ti);
    strftime(cur.updatedStr, sizeof(cur.updatedStr), "%I:%M:%S %p", &ti);
    if (cur.updatedStr[0] == '0')
      memmove(cur.updatedStr, cur.updatedStr + 1, strlen(cur.updatedStr));
  } else {
    strncpy(cur.updatedStr, "--", sizeof(cur.updatedStr) - 1);
    cur.updatedStr[sizeof(cur.updatedStr) - 1] = '\0';
  }

  snprintf(cur.tempStr, sizeof(cur.tempStr), "%.0f%s", cur.temp,
           tempSuffix(s.tempUnit));
  snprintf(cur.humidityStr, sizeof(cur.humidityStr), "H: %.0f%%", cur.humidity);
  snprintf(cur.windStr, sizeof(cur.windStr), "W: %.0f%s", cur.wind,
           windSuffix(s.windUnit));
  snprintf(cur.precipStr, sizeof(cur.precipStr), "P: %.1f%s", cur.precip,
           precipSuffix(s.precipUnit));
  strncpy(
      cur.conditionStr,
      getCondition(cur.weatherCode, cur.precip, cur.humidity, cur.isDay == 1),
      sizeof(cur.conditionStr) - 1);

  JsonObject hourly = doc["hourly"];
  JsonArray hTime = hourly["time"];
  JsonArray hTemp = hourly["temperature_2m"];
  JsonArray hPrecipProb = hourly["precipitation_probability"];
  JsonArray hPrecip = hourly["precipitation"];
  JsonArray hHumidity = hourly["relative_humidity_2m"];
  JsonArray hWeatherCode = hourly["weather_code"];
  JsonArray hWind = hourly["wind_speed_10m"];

  if (cur.observedAt <= 0 && hTime.size() > 0) {
    cur.observedAt = (int32_t)hTime[0];
  }

  size_t hourlyCount = min({hTime.size(), hTemp.size(), hPrecipProb.size(),
                            hPrecip.size(), hHumidity.size(), hWind.size(),
                            hWeatherCode.size(), (size_t)WEATHER_MAX_HOURLY});

  JsonObject daily = doc["daily"];
  JsonArray dSunrise = daily["sunrise"];
  JsonArray dSunset = daily["sunset"];
  int32_t todaySunrise = dSunrise.size() > 0 ? (int32_t)dSunrise[0] : 0;
  int32_t todaySunset = dSunset.size() > 0 ? (int32_t)dSunset[0] : 0;

  static HourlyData hourlyData[WEATHER_MAX_HOURLY];
  for (size_t i = 0; i < hourlyCount; i++) {
    HourlyData &h = hourlyData[i];
    h.time = hTime[i];
    h.temp = hTemp[i];
    h.precipProb = hPrecipProb[i];
    h.precip = hPrecip[i];
    h.humidity = hHumidity[i];
    h.wind = hWind[i];
    h.weatherCode = hWeatherCode[i] | -1;

    formatTime(h.time, "%I%p", h.timeStr, sizeof(h.timeStr));
    if (h.timeStr[0] == '0')
      memmove(h.timeStr, h.timeStr + 1, strlen(h.timeStr));
    size_t tlen = strlen(h.timeStr);
    if (tlen >= 2) {
      char ampm[3] = {h.timeStr[tlen - 2], h.timeStr[tlen - 1], '\0'};
      snprintf(h.timeStr, sizeof(h.timeStr), "%.*s %s", (int)(tlen - 2),
               h.timeStr, ampm);
    }

    snprintf(h.tempStr, sizeof(h.tempStr), "%.0f", h.temp);
    snprintf(h.precipStr, sizeof(h.precipStr), "%.0f%%", h.precipProb);

    bool isDay = (h.time >= todaySunrise && h.time < todaySunset);
    strncpy(h.conditionStr, getHourlyCondition(h, isDay),
            sizeof(h.conditionStr) - 1);
  }

  JsonArray dTime = daily["time"];
  JsonArray dTempMax = daily["temperature_2m_max"];
  JsonArray dTempMin = daily["temperature_2m_min"];
  JsonArray dUV = daily["uv_index_max"];
  JsonArray dPrecipSum = daily["precipitation_sum"];
  JsonArray dWeatherCode = daily["weather_code"];

  size_t dailyCount =
      min({dTime.size(), dTempMax.size(), dTempMin.size(), dSunrise.size(),
           dSunset.size(), dUV.size(), dPrecipSum.size(), dWeatherCode.size(),
           (size_t)WEATHER_MAX_DAILY});

  static DailyData dailyData[WEATHER_MAX_DAILY];
  for (size_t i = 0; i < dailyCount; i++) {
    DailyData &d = dailyData[i];
    d.time = dTime[i];
    d.sunrise = dSunrise[i];
    d.sunset = dSunset[i];
    d.tempMax = dTempMax[i];
    d.tempMin = dTempMin[i];
    d.uvIndex = dUV[i];
    d.precipSum = dPrecipSum[i];
    d.weatherCode = dWeatherCode[i] | -1;

    formatTime(d.time, "%A", d.dayStr, sizeof(d.dayStr));
    snprintf(d.tempRangeStr, sizeof(d.tempRangeStr), "%.0f / %.0f", d.tempMax,
             d.tempMin);
    snprintf(d.precipStr, sizeof(d.precipStr), "%.1f%s", d.precipSum,
             precipSuffix(s.precipUnit));
    strncpy(d.conditionStr, getDailyCondition(d), sizeof(d.conditionStr) - 1);
    formatTime(d.sunrise, "%I:%M%p", d.sunriseStr, sizeof(d.sunriseStr));
    formatTime(d.sunset, "%I:%M%p", d.sunsetStr, sizeof(d.sunsetStr));
  }

  Data.setWeather(cur, hourlyData, hourlyCount, dailyData, dailyCount);
  WebConfig::logPrintf("[Weather] %.0f%s, %s (%uh, %lums)\n", cur.temp,
                       tempSuffix(s.tempUnit), cur.conditionStr,
                       (unsigned)hourlyCount, (unsigned long)latency);
}

} // namespace Weather
