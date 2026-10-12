#pragma once

#include "Config.h"
#include "DataManager.h"
#include "WebConfig.h"
#include "Secrets.h"
#include <Arduino.h>
#include <ArduinoOTA.h>
#include <WiFi.h>
#include <string.h>
#include <time.h>

namespace Net {

static bool hasValue(const char *s) { return s && s[0] != '\0'; }

static void copyCstr(char *dst, size_t len, const char *src) {
  if (!dst || len == 0)
    return;
  if (!src)
    src = "";
  strncpy(dst, src, len - 1);
  dst[len - 1] = '\0';
}

static void ensureWifiDefaults(Settings &s) {
  if (hasValue(s.wifiSsid))
    return;
  if (!hasValue(WIFI_SSID))
    return;

  copyCstr(s.wifiSsid, sizeof(s.wifiSsid), WIFI_SSID);
  copyCstr(s.wifiPass, sizeof(s.wifiPass), WIFI_PASS);
  Data.setSettings(s);
  SettingsStore.save(s);
  Serial.printf("[Network] Using WiFi defaults from Secrets.h: %s\n",
                s.wifiSsid);
}

static bool connect(const char *ssid, const char *pass, uint32_t timeout) {
  if (!ssid || ssid[0] == '\0') {
    Data.setWifi(false);
    return false;
  }

  WiFi.mode(WIFI_STA);
  WiFi.disconnect(true, true);
  delay(100);
  WiFi.begin(ssid, pass);

  uint32_t start = millis();
  while (WiFi.status() != WL_CONNECTED && millis() - start < timeout) {
    delay(250);
  }

  bool ok = WiFi.status() == WL_CONNECTED;
  Data.setWifi(ok, ok ? WiFi.RSSI() : 0);
  if (ok) {
    Serial.printf("[Network] Connected: %s (%ddBm)\n", ssid, WiFi.RSSI());
  } else {
    Serial.printf("[Network] Connect failed: %s\n", ssid);
  }
  return ok;
}

static bool connectFromSettings(Settings &s) {
  ensureWifiDefaults(s);
  bool ok = connect(s.wifiSsid, s.wifiPass, WIFI_CONNECT_TIMEOUT_MS);

  if (!ok && hasValue(WIFI_SSID) && strcmp(s.wifiSsid, WIFI_SSID) != 0) {
    Serial.printf("[Network] Trying fallback WiFi from Secrets.h: %s\n",
                  WIFI_SSID);
    ok = connect(WIFI_SSID, WIFI_PASS, WIFI_CONNECT_TIMEOUT_MS);
    if (ok) {
      copyCstr(s.wifiSsid, sizeof(s.wifiSsid), WIFI_SSID);
      copyCstr(s.wifiPass, sizeof(s.wifiPass), WIFI_PASS);
      Data.setSettings(s);
      SettingsStore.save(s);
    }
  }
  return ok;
}

void init() {
  Settings s = Data.getSettings();
  connectFromSettings(s);

  configTime(0, 0, NTP_SERVER);
  setenv("TZ", TIMEZONE, 1);
  tzset();

  ArduinoOTA.setHostname(CLOCK_NAME);
  ArduinoOTA.setPassword(OTA_PASSWORD);
  ArduinoOTA.onStart([]() { Serial.println("[OTA] Update started..."); });
  ArduinoOTA.onEnd([]() { Serial.println("[OTA] Update complete!"); });
  ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
    Serial.printf("[OTA] Progress: %u%%\n", (progress * 100) / total);
  });
  ArduinoOTA.onError([](ota_error_t err) {
    Serial.printf("[OTA] Error[%u]: ", err);
    if (err == OTA_AUTH_ERROR) Serial.println("Auth failed");
    else if (err == OTA_BEGIN_ERROR) Serial.println("Begin failed");
    else if (err == OTA_CONNECT_ERROR) Serial.println("Connect failed");
    else if (err == OTA_RECEIVE_ERROR) Serial.println("Receive failed");
    else if (err == OTA_END_ERROR) Serial.println("End failed");
  });
  ArduinoOTA.begin();
  WebConfig::begin();
  Serial.println("[Network] Initialized");
}

void updateTime() {
  static uint32_t lastUpdate = 0;
  if (millis() - lastUpdate < 250)
    return;
  lastUpdate = millis();

  char t[16], d[24], ts[8], ap[4];
  time_t now = time(nullptr);

  if (now > 100000) {
    struct tm ti;
    localtime_r(&now, &ti);
    strftime(t, sizeof(t), "%I:%M:%S %p", &ti);
    if (t[0] == '0')
      memmove(t, t + 1, strlen(t));
    strftime(ts, sizeof(ts), "%I:%M", &ti);
    if (ts[0] == '0')
      memmove(ts, ts + 1, strlen(ts));
    strftime(ap, sizeof(ap), "%p", &ti);
    strftime(d, sizeof(d), "%a, %b %d", &ti);
  } else {
    uint32_t s = millis() / 1000;
    uint32_t hh = (s / 3600) % 24;
    uint32_t mm = (s / 60) % 60;
    uint32_t ss = s % 60;
    uint32_t hh12 = hh % 12;
    if (hh12 == 0)
      hh12 = 12;
    const char *ampm = hh < 12 ? "AM" : "PM";
    snprintf(t, sizeof(t), "%lu:%02lu:%02lu %s", (unsigned long)hh12,
             (unsigned long)mm, (unsigned long)ss, ampm);
    snprintf(ts, sizeof(ts), "%lu:%02lu", (unsigned long)hh12,
             (unsigned long)mm);
    snprintf(ap, sizeof(ap), "%s", ampm);
    snprintf(d, sizeof(d), "Syncing time...");
  }

  Data.setTime(t, d, ts, ap);
}

void loop() {
  ArduinoOTA.handle();
  WebConfig::handle();

  static uint32_t lastCheck = 0;
  if (millis() - lastCheck > WIFI_CHECK_INTERVAL_MS) {
    bool connected = WiFi.status() == WL_CONNECTED;
    Data.setWifi(connected, connected ? WiFi.RSSI() : 0);

    if (!connected) {
      Settings s = Data.getSettings();
      connectFromSettings(s);
    }
    lastCheck = millis();
  }

  if (Data.consumeWifiReconnect()) {
    Settings s = Data.getSettings();
    connectFromSettings(s);
  }

  updateTime();
}

} // namespace Net
