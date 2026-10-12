#pragma once

#include "Backlight.h"
#include "Config.h"
#include "DataManager.h"
#include "Theme.h"
#include "WeatherVisuals.h"
#include "WebConfig.h"
#include <Arduino.h>
#include <lvgl.h>
#include <math.h>
#include <string.h>
#include <time.h>

LV_FONT_DECLARE(lv_font_clock_96);
LV_FONT_DECLARE(lv_font_temp_64);

namespace UI {

static lv_obj_t *homeScreen = nullptr;
static lv_obj_t *forecastScreen = nullptr;

static lv_obj_t *heroBgImage = nullptr;
static lv_obj_t *heroSunGlow = nullptr;
static lv_obj_t *heroMoonGlow = nullptr;

static constexpr int HERO_CLOUD_NEAR = 3;
static constexpr int HERO_CLOUD_FAR = 2;
static constexpr int HERO_RAIN_COUNT = 18;
static constexpr int HERO_SNOW_COUNT = 20;
static constexpr int HERO_STAR_COUNT = 24;
static constexpr int HERO_FOG_COUNT = 3;
static constexpr int HERO_BIRD_COUNT = 5;

static lv_obj_t *heroCloudAnim[HERO_CLOUD_NEAR + HERO_CLOUD_FAR] = {nullptr};
static lv_obj_t *heroRainAnim[HERO_RAIN_COUNT] = {nullptr};
static lv_obj_t *heroStarAnim[HERO_STAR_COUNT] = {nullptr};
static lv_obj_t *heroSnowAnim[HERO_SNOW_COUNT] = {nullptr};
static lv_obj_t *heroFogAnim[HERO_FOG_COUNT] = {nullptr};
static lv_obj_t *heroBirdAnim[HERO_BIRD_COUNT] = {nullptr};
static lv_obj_t *heroShoot = nullptr;
static lv_obj_t *heroFlashOverlay = nullptr;
static lv_obj_t *heroBolt = nullptr;

// Weather-driven motion: cloud drift scales with wind, rain/snow density and
// fall speed with precipitation, fog opacity with humidity. Recomputed only
// when new weather arrives (updateHero) and consumed every animation tick.
static float animWindFactor = 1.0f;
static uint8_t animRainCount = 14;
static uint8_t animSnowCount = 18;
static uint8_t animFogOpa = 150;
static int animRainSpeedDiv = 3;
static bool shootActive = false;
static uint32_t shootStartMs = 0;
static uint32_t shootNextMs = 3000;
static int16_t shootX0 = 0;
static int16_t shootY0 = 0;
static uint32_t shootRng = 0x9E3779B9u;

static lv_obj_t *lblTime = nullptr;
static lv_obj_t *lblAmPm = nullptr;
static lv_obj_t *lblDate = nullptr;
static lv_obj_t *lblTemp = nullptr;
static lv_obj_t *lblCondition = nullptr;
static lv_obj_t *lblMetrics = nullptr;
static lv_obj_t *lblChipWifi = nullptr;
static lv_obj_t *lblChipIndoor = nullptr;
static lv_obj_t *lblChipUpdated = nullptr;
static lv_obj_t *btnSettings = nullptr;
static lv_obj_t *btnRefresh = nullptr;
static lv_obj_t *toastLabel = nullptr;
static lv_obj_t *dotHome[2] = {nullptr};
static lv_obj_t *dotForecast[2] = {nullptr};

static constexpr int HOURLY_SLOTS = 8;
static lv_obj_t *hourlyPanel = nullptr;
static lv_obj_t *hourlyCards[HOURLY_SLOTS] = {nullptr};
static lv_obj_t *hourlyTime[HOURLY_SLOTS] = {nullptr};
static lv_obj_t *hourlyIcon[HOURLY_SLOTS] = {nullptr};
static lv_obj_t *hourlyTemp[HOURLY_SLOTS] = {nullptr};

static lv_obj_t *dailyCont = nullptr;
static lv_obj_t *dailyCards[7] = {nullptr};
static lv_obj_t *dailyDay[7] = {nullptr};
static lv_obj_t *dailyIcon[7] = {nullptr};
static lv_obj_t *dailyTemp[7] = {nullptr};
static lv_obj_t *dailyPrecip[7] = {nullptr};

static lv_obj_t *trendCard = nullptr;
static lv_obj_t *trendChart = nullptr;
static lv_chart_series_t *trendTempSeries = nullptr;
static lv_chart_series_t *trendPrecipSeries = nullptr;
static lv_obj_t *lblTrendSummary = nullptr;

static lv_obj_t *lblSunrise = nullptr;
static lv_obj_t *lblSunset = nullptr;

static lv_obj_t *lblPageHint = nullptr;
static lv_obj_t *settingsModal = nullptr;
static lv_obj_t *sliderBrightness = nullptr;
static lv_obj_t *lblBrightness = nullptr;
static lv_obj_t *ddTemp = nullptr;
static lv_obj_t *ddWind = nullptr;
static lv_obj_t *ddPrecip = nullptr;
static lv_obj_t *swNight = nullptr;
static lv_obj_t *ddNightStart = nullptr;
static lv_obj_t *ddNightEnd = nullptr;
static lv_obj_t *sliderNight = nullptr;
static lv_obj_t *lblNightBright = nullptr;
static lv_obj_t *lblDiagFps = nullptr;
static lv_obj_t *lblDiagMem = nullptr;
static lv_obj_t *lblDiagWifi = nullptr;
static lv_obj_t *lblDiagApi = nullptr;

static CurrentWeather curCache;
static HourlyData hourlyCache[WEATHER_MAX_HOURLY];
static DailyData dailyCache[WEATHER_MAX_DAILY];
static size_t hourlyCount = 0;
static size_t dailyCount = 0;
static uint32_t lastWeatherVer = 0;
static uint32_t lastDiagVer = 0;
static uint32_t lastSettingsVer = 0;
static Settings settingsCache;
static DiagnosticsData diagCache;
static uint32_t lastAnimMs = 0;
static uint32_t nextThunderFlashMs = 0;
static uint32_t thunderFlashStartMs = 0;
static uint32_t thunderFlashEndMs = 0;
static WeatherVisuals::VisualMode heroMode = WeatherVisuals::VIS_SUN;
#if ENABLE_BME280
static IndoorData indoorCache;
static uint32_t lastIndoorVer = 0;
#endif

static void closeSettings(lv_event_t *e) {
  if (settingsModal) {
    WebConfig::logPrintf("[UI] Settings modal closed\n");
    lv_obj_delete(settingsModal);
    settingsModal = nullptr;
    sliderBrightness = nullptr;
    lblBrightness = nullptr;
    ddTemp = nullptr;
    ddWind = nullptr;
    ddPrecip = nullptr;
    swNight = nullptr;
    ddNightStart = nullptr;
    ddNightEnd = nullptr;
    sliderNight = nullptr;
    lblNightBright = nullptr;
    lblDiagFps = nullptr;
    lblDiagMem = nullptr;
    lblDiagWifi = nullptr;
    lblDiagApi = nullptr;
  }
}

static void onBackdropClick(lv_event_t *e) {
  // Only close when the backdrop itself is tapped, not a child of the panel.
  if ((lv_obj_t *)lv_event_get_target(e) == settingsModal)
    closeSettings(e);
}

static void updateDiagnosticsPanel() {
  if (!settingsModal || !lblDiagFps || !lblDiagMem || !lblDiagWifi ||
      !lblDiagApi)
    return;

  char buf[160];
  snprintf(buf, sizeof(buf), "FPS: %.1f", diagCache.fps);
  lv_label_set_text(lblDiagFps, buf);

  if (diagCache.psramTotal > 0) {
    snprintf(
        buf, sizeof(buf), "Heap: %lu/%lu KB | PSRAM: %lu/%lu KB",
        (unsigned long)((diagCache.heapTotal - diagCache.heapFree) / 1024),
        (unsigned long)(diagCache.heapTotal / 1024),
        (unsigned long)((diagCache.psramTotal - diagCache.psramFree) / 1024),
        (unsigned long)(diagCache.psramTotal / 1024));
  } else {
    snprintf(buf, sizeof(buf), "Heap: %lu/%lu KB",
             (unsigned long)((diagCache.heapTotal - diagCache.heapFree) / 1024),
             (unsigned long)(diagCache.heapTotal / 1024));
  }
  lv_label_set_text(lblDiagMem, buf);

  if (diagCache.wifiConnected) {
    snprintf(buf, sizeof(buf), "WiFi: %ddBm (%u%%)", diagCache.wifiRssi,
             diagCache.wifiQuality);
  } else {
    snprintf(buf, sizeof(buf), "WiFi: Offline");
  }
  lv_label_set_text(lblDiagWifi, buf);

  snprintf(buf, sizeof(buf), "API: Weather %lums %s",
           (unsigned long)diagCache.weatherLatencyMs,
           diagCache.weatherOk ? "OK" : "ERR");
  lv_label_set_text(lblDiagApi, buf);
}

static void onBrightnessChanged(lv_event_t *e) {
  lv_obj_t *slider = (lv_obj_t *)lv_event_get_target(e);
  int val = lv_slider_get_value(slider);
  Backlight::set(val);

  if (lblBrightness) {
    char buf[24];
    snprintf(buf, sizeof(buf), "%d%%", (val * 100) / BACKLIGHT_MAX);
    lv_label_set_text(lblBrightness, buf);
  }

  if (lv_event_get_code(e) == LV_EVENT_RELEASED) {
    settingsCache.brightness = val;
    Data.setSettings(settingsCache);
    SettingsStore.save(settingsCache);
  }
}

static void onUnitChanged(lv_event_t *e) {
  lv_obj_t *dd = (lv_obj_t *)lv_event_get_target(e);
  uint16_t sel = lv_roller_get_selected(dd);

  if (dd == ddTemp)
    settingsCache.tempUnit = (TempUnit)sel;
  else if (dd == ddWind)
    settingsCache.windUnit = (WindUnit)sel;
  else if (dd == ddPrecip)
    settingsCache.precipUnit = (PrecipUnit)sel;

  Data.setSettings(settingsCache);
  SettingsStore.save(settingsCache);
  Data.requestWeatherRefresh();
}

static void saveSettingsAndApplyNight() {
  Data.setSettings(settingsCache);
  SettingsStore.save(settingsCache);
  Backlight::setNightMode(isNightModeActiveNow(settingsCache),
                          settingsCache.nightBrightness);
}

static void onNightToggle(lv_event_t *e) {
  lv_obj_t *sw = (lv_obj_t *)lv_event_get_target(e);
  settingsCache.nightModeEnabled = lv_obj_has_state(sw, LV_STATE_CHECKED);
  saveSettingsAndApplyNight();
}

static void onNightHourChanged(lv_event_t *e) {
  lv_obj_t *dd = (lv_obj_t *)lv_event_get_target(e);
  uint16_t sel = lv_roller_get_selected(dd);

  if (dd == ddNightStart)
    settingsCache.nightStartHour = (uint8_t)sel;
  else if (dd == ddNightEnd)
    settingsCache.nightEndHour = (uint8_t)sel;

  saveSettingsAndApplyNight();
}

static void onNightBrightChanged(lv_event_t *e) {
  lv_obj_t *slider = (lv_obj_t *)lv_event_get_target(e);
  int val = lv_slider_get_value(slider);

  if (lblNightBright) {
    char buf[16];
    snprintf(buf, sizeof(buf), "%d%%", (val * 100) / BACKLIGHT_MAX);
    lv_label_set_text(lblNightBright, buf);
  }

  if (lv_event_get_code(e) == LV_EVENT_RELEASED) {
    settingsCache.nightBrightness = (uint8_t)val;
    saveSettingsAndApplyNight();
  }
}

// ---- toast + page navigation ----------------------------------------
static uint32_t toastUntilMs = 0;

static void showToast(const char *msg, uint32_t ms = 2500) {
  if (!toastLabel)
    return;
  lv_label_set_text(toastLabel, msg);
  lv_obj_remove_flag(toastLabel, LV_OBJ_FLAG_HIDDEN);
  toastUntilMs = millis() + ms;
}

static void updatePageDots(bool onForecast) {
  for (int i = 0; i < 2; i++) {
    lv_opa_t on = (i == (onForecast ? 1 : 0)) ? LV_OPA_90 : LV_OPA_30;
    if (dotHome[i])
      lv_obj_set_style_bg_opa(dotHome[i], on, 0);
    if (dotForecast[i])
      lv_obj_set_style_bg_opa(dotForecast[i], on, 0);
  }
}

static void showForecast() {
  if (!forecastScreen || lv_screen_active() == forecastScreen)
    return;
  updatePageDots(true);
  lv_screen_load_anim(forecastScreen, LV_SCREEN_LOAD_ANIM_MOVE_TOP, 260, 0, false);
}

static void showHome() {
  if (!homeScreen || lv_screen_active() == homeScreen)
    return;
  updatePageDots(false);
  lv_screen_load_anim(homeScreen, LV_SCREEN_LOAD_ANIM_MOVE_BOTTOM, 260, 0, false);
}

static void onHomeGesture(lv_event_t *e) {
  if (lv_indev_get_gesture_dir(lv_indev_active()) == LV_DIR_TOP)
    showForecast();
}

static void onForecastGesture(lv_event_t *e) {
  if (lv_indev_get_gesture_dir(lv_indev_active()) == LV_DIR_BOTTOM)
    showHome();
}

static void onDotClicked(lv_event_t *e) {
  if (lv_event_get_user_data(e))
    showForecast();
  else
    showHome();
}

static void onRefreshWeather(lv_event_t *e) {
  Data.requestWeatherRefresh();
  showToast(LV_SYMBOL_REFRESH "  Refreshing weather...");
}

static lv_obj_t *addSectionHeader(lv_obj_t *parent, const char *text) {
  lv_obj_t *header = lv_label_create(parent);
  lv_label_set_text(header, text);
  lv_obj_set_style_text_font(header, &lv_font_montserrat_20, 0);
  lv_obj_set_style_text_color(header, Theme::COLOR_ACCENT, 0);
  return header;
}

static lv_obj_t *addRow(lv_obj_t *parent) {
  lv_obj_t *row = lv_obj_create(parent);
  lv_obj_set_size(row, LV_PCT(100), LV_SIZE_CONTENT);
  lv_obj_set_flex_flow(row, LV_FLEX_FLOW_ROW);
  lv_obj_set_flex_align(row, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER,
                        LV_FLEX_ALIGN_CENTER);
  lv_obj_set_style_bg_opa(row, LV_OPA_TRANSP, 0);
  lv_obj_set_style_border_width(row, 0, 0);
  lv_obj_set_style_pad_all(row, 0, 0);
  lv_obj_set_style_pad_column(row, 10, 0);
  lv_obj_clear_flag(row, LV_OBJ_FLAG_SCROLLABLE);
  return row;
}

static lv_obj_t *addRowLabel(lv_obj_t *row, const char *text) {
  lv_obj_t *lbl = lv_label_create(row);
  lv_label_set_text(lbl, text);
  lv_obj_set_style_text_color(lbl, Theme::COLOR_TEXT, 0);
  return lbl;
}

static const char *HOUR_OPTIONS =
    "12 AM\n1 AM\n2 AM\n3 AM\n4 AM\n5 AM\n6 AM\n7 AM\n8 AM\n9 AM\n10 AM\n"
    "11 AM\n12 PM\n1 PM\n2 PM\n3 PM\n4 PM\n5 PM\n6 PM\n7 PM\n8 PM\n9 PM\n"
    "10 PM\n11 PM";

static void showSettings() {
  if (settingsModal)
    return;

  // Full-screen dimmed backdrop: blocks touches to the main screen and
  // closes the modal when tapped outside the panel.
  settingsModal = lv_obj_create(lv_layer_top());
  lv_obj_set_size(settingsModal, SCREEN_WIDTH, SCREEN_HEIGHT);
  lv_obj_set_pos(settingsModal, 0, 0);
  lv_obj_set_style_bg_color(settingsModal, lv_color_hex(0x000000), 0);
  lv_obj_set_style_bg_opa(settingsModal, LV_OPA_60, 0);
  lv_obj_set_style_border_width(settingsModal, 0, 0);
  lv_obj_set_style_radius(settingsModal, 0, 0);
  lv_obj_set_style_pad_all(settingsModal, 0, 0);
  lv_obj_clear_flag(settingsModal, LV_OBJ_FLAG_SCROLLABLE);
  lv_obj_add_event_cb(settingsModal, onBackdropClick, LV_EVENT_CLICKED,
                      nullptr);

  lv_obj_t *panel = lv_obj_create(settingsModal);
  lv_obj_set_size(panel, 640, 440);
  lv_obj_center(panel);
  lv_obj_add_style(panel, &Theme::stylePanel, 0);
  lv_obj_set_flex_flow(panel, LV_FLEX_FLOW_COLUMN);
  lv_obj_set_style_pad_row(panel, 8, 0);
  lv_obj_add_flag(panel, LV_OBJ_FLAG_SCROLLABLE);
  lv_obj_set_scroll_dir(panel, LV_DIR_VER);

  // --- Header: title left, close button right ---
  lv_obj_t *headerRow = addRow(panel);
  lv_obj_set_flex_align(headerRow, LV_FLEX_ALIGN_SPACE_BETWEEN,
                        LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);

  lv_obj_t *title = lv_label_create(headerRow);
  lv_label_set_text(title, "Settings  " CLOCK_VERSION);
  lv_obj_set_style_text_font(title, &lv_font_montserrat_24, 0);
  lv_obj_set_style_text_color(title, Theme::COLOR_TEXT, 0);

  lv_obj_t *closeBtn = lv_btn_create(headerRow);
  lv_obj_set_size(closeBtn, 48, 48);
  lv_obj_add_event_cb(closeBtn, closeSettings, LV_EVENT_CLICKED, nullptr);
  lv_obj_t *x = lv_label_create(closeBtn);
  lv_label_set_text(x, LV_SYMBOL_CLOSE);
  lv_obj_center(x);

  // --- Display ---
  addSectionHeader(panel, "Display");

  lv_obj_t *brightRow = addRow(panel);
  addRowLabel(brightRow, "Brightness:");

  sliderBrightness = lv_slider_create(brightRow);
  lv_slider_set_range(sliderBrightness, 10, BACKLIGHT_MAX);
  lv_slider_set_value(sliderBrightness, Backlight::get(), LV_ANIM_OFF);
  lv_obj_set_width(sliderBrightness, 260);
  lv_obj_add_event_cb(sliderBrightness, onBrightnessChanged,
                      LV_EVENT_VALUE_CHANGED, nullptr);
  lv_obj_add_event_cb(sliderBrightness, onBrightnessChanged, LV_EVENT_RELEASED,
                      nullptr);

  lblBrightness = lv_label_create(brightRow);
  char buf[16];
  snprintf(buf, sizeof(buf), "%d%%", (Backlight::get() * 100) / BACKLIGHT_MAX);
  lv_label_set_text(lblBrightness, buf);
  lv_obj_set_style_text_color(lblBrightness, Theme::COLOR_SUBTEXT, 0);

  // --- Night mode ---
  addSectionHeader(panel, "Night Mode");

  lv_obj_t *nightRow = addRow(panel);
  addRowLabel(nightRow, "Dim display at night:");

  swNight = lv_switch_create(nightRow);
  if (settingsCache.nightModeEnabled)
    lv_obj_add_state(swNight, LV_STATE_CHECKED);
  lv_obj_add_event_cb(swNight, onNightToggle, LV_EVENT_VALUE_CHANGED, nullptr);

  lv_obj_t *nightHoursRow = addRow(panel);
  addRowLabel(nightHoursRow, "From:");

  ddNightStart = lv_roller_create(nightHoursRow);
  lv_roller_set_options(ddNightStart, HOUR_OPTIONS, LV_ROLLER_MODE_NORMAL);
  lv_roller_set_visible_row_count(ddNightStart, 3);
  lv_obj_set_width(ddNightStart, 110);
  lv_roller_set_selected(ddNightStart, settingsCache.nightStartHour,
                         LV_ANIM_OFF);
  lv_obj_add_event_cb(ddNightStart, onNightHourChanged, LV_EVENT_VALUE_CHANGED,
                      nullptr);

  addRowLabel(nightHoursRow, "To:");

  ddNightEnd = lv_roller_create(nightHoursRow);
  lv_roller_set_options(ddNightEnd, HOUR_OPTIONS, LV_ROLLER_MODE_NORMAL);
  lv_roller_set_visible_row_count(ddNightEnd, 3);
  lv_obj_set_width(ddNightEnd, 110);
  lv_roller_set_selected(ddNightEnd, settingsCache.nightEndHour, LV_ANIM_OFF);
  lv_obj_add_event_cb(ddNightEnd, onNightHourChanged, LV_EVENT_VALUE_CHANGED,
                      nullptr);

  lv_obj_t *nightBrightRow = addRow(panel);
  addRowLabel(nightBrightRow, "Night brightness:");

  sliderNight = lv_slider_create(nightBrightRow);
  lv_slider_set_range(sliderNight, 10, BACKLIGHT_MAX);
  lv_slider_set_value(sliderNight, settingsCache.nightBrightness, LV_ANIM_OFF);
  lv_obj_set_width(sliderNight, 200);
  lv_obj_add_event_cb(sliderNight, onNightBrightChanged,
                      LV_EVENT_VALUE_CHANGED, nullptr);
  lv_obj_add_event_cb(sliderNight, onNightBrightChanged, LV_EVENT_RELEASED,
                      nullptr);

  lblNightBright = lv_label_create(nightBrightRow);
  snprintf(buf, sizeof(buf), "%d%%",
           (settingsCache.nightBrightness * 100) / BACKLIGHT_MAX);
  lv_label_set_text(lblNightBright, buf);
  lv_obj_set_style_text_color(lblNightBright, Theme::COLOR_SUBTEXT, 0);

  // --- Units ---
  addSectionHeader(panel, "Units");

  lv_obj_t *unitRow = addRow(panel);
  addRowLabel(unitRow, "Temp/Wind/Precip:");

  ddTemp = lv_roller_create(unitRow);
  lv_roller_set_options(ddTemp, "F\nC", LV_ROLLER_MODE_NORMAL);
  lv_roller_set_visible_row_count(ddTemp, 2);
  lv_obj_set_width(ddTemp, 65);
  lv_roller_set_selected(ddTemp, settingsCache.tempUnit, LV_ANIM_OFF);
  lv_obj_add_event_cb(ddTemp, onUnitChanged, LV_EVENT_VALUE_CHANGED, nullptr);

  ddWind = lv_roller_create(unitRow);
  lv_roller_set_options(ddWind, "mph\nkm/h", LV_ROLLER_MODE_NORMAL);
  lv_roller_set_visible_row_count(ddWind, 2);
  lv_obj_set_width(ddWind, 90);
  lv_roller_set_selected(ddWind, settingsCache.windUnit, LV_ANIM_OFF);
  lv_obj_add_event_cb(ddWind, onUnitChanged, LV_EVENT_VALUE_CHANGED, nullptr);

  ddPrecip = lv_roller_create(unitRow);
  lv_roller_set_options(ddPrecip, "in\nmm", LV_ROLLER_MODE_NORMAL);
  lv_roller_set_visible_row_count(ddPrecip, 2);
  lv_obj_set_width(ddPrecip, 65);
  lv_roller_set_selected(ddPrecip, settingsCache.precipUnit, LV_ANIM_OFF);
  lv_obj_add_event_cb(ddPrecip, onUnitChanged, LV_EVENT_VALUE_CHANGED, nullptr);

  // --- Data ---
  addSectionHeader(panel, "Data");

  lv_obj_t *dataRow = addRow(panel);
  lv_obj_t *refreshBtn = lv_btn_create(dataRow);
  lv_obj_add_style(refreshBtn, &Theme::styleBtn, 0);
  lv_obj_add_event_cb(refreshBtn, onRefreshWeather, LV_EVENT_CLICKED, nullptr);
  lv_obj_t *refreshLbl = lv_label_create(refreshBtn);
  lv_label_set_text(refreshLbl, LV_SYMBOL_REFRESH "  Refresh Weather");
  lv_obj_center(refreshLbl);

  // --- Debug ---
  addSectionHeader(panel, "Debug");

  lblDiagFps = lv_label_create(panel);
  lv_obj_set_style_text_color(lblDiagFps, Theme::COLOR_SUBTEXT, 0);
  lv_label_set_text(lblDiagFps, "FPS: --");

  lblDiagMem = lv_label_create(panel);
  lv_obj_set_width(lblDiagMem, LV_PCT(100));
  lv_label_set_long_mode(lblDiagMem, LV_LABEL_LONG_WRAP);
  lv_obj_set_style_text_color(lblDiagMem, Theme::COLOR_SUBTEXT, 0);
  lv_label_set_text(lblDiagMem, "Heap: --");

  lblDiagWifi = lv_label_create(panel);
  lv_obj_set_style_text_color(lblDiagWifi, Theme::COLOR_SUBTEXT, 0);
  lv_label_set_text(lblDiagWifi, "WiFi: --");

  lblDiagApi = lv_label_create(panel);
  lv_obj_set_width(lblDiagApi, LV_PCT(100));
  lv_label_set_long_mode(lblDiagApi, LV_LABEL_LONG_WRAP);
  lv_obj_set_style_text_color(lblDiagApi, Theme::COLOR_SUBTEXT, 0);
  lv_label_set_text(lblDiagApi, "API: --");

  // --- Web config ---
  addSectionHeader(panel, "Web Config / OTA");

  lv_obj_t *otaUrl = lv_label_create(panel);
  NetworkState otaNet = Data.getNetwork();
  char otaBuf[64];
  if (otaNet.connected) {
    snprintf(otaBuf, sizeof(otaBuf), "http://%s/",
             WiFi.localIP().toString().c_str());
  } else {
    snprintf(otaBuf, sizeof(otaBuf), "Not connected");
  }
  lv_label_set_text(otaUrl, otaBuf);
  lv_obj_set_width(otaUrl, LV_PCT(100));
  lv_label_set_long_mode(otaUrl, LV_LABEL_LONG_WRAP);
  lv_obj_set_style_text_color(otaUrl, Theme::COLOR_SUBTEXT, 0);

  updateDiagnosticsPanel();
  WebConfig::logPrintf("[UI] Settings modal open\n");
}

static void onSettingsClick(lv_event_t *e) {
  WebConfig::logPrintf("[UI] Settings button tapped\n");
  showSettings();
}

static WeatherVisuals::VisualMode
dailyModeForHour(int32_t epoch, const char *cond, float precip) {
  bool isDay = curCache.isDay == 1;
  if (dailyCount > 0 && dailyCache[0].sunrise > 0 && dailyCache[0].sunset > 0 &&
      epoch > 0) {
    isDay = (epoch >= dailyCache[0].sunrise && epoch < dailyCache[0].sunset);
  }
  return WeatherVisuals::fromCondition(cond, isDay, precip);
}

static void setHeroMode(WeatherVisuals::VisualMode mode) {
  heroMode = mode;
  if (heroBgImage) {
    const lv_image_dsc_t *bg = WeatherVisuals::background(mode);
    if (bg)
      lv_image_set_src(heroBgImage, bg);
  }
}

// Hide/show an effect object only when the state actually changes, so
// inactive effects cost zero invalidations (hidden objects are skipped by
// LVGL entirely; transparent-but-visible ones are not).
static void setEffectVisible(lv_obj_t *obj, bool show) {
  if (!obj)
    return;
  bool hidden = lv_obj_has_flag(obj, LV_OBJ_FLAG_HIDDEN);
  if (show && hidden)
    lv_obj_remove_flag(obj, LV_OBJ_FLAG_HIDDEN);
  else if (!show && !hidden)
    lv_obj_add_flag(obj, LV_OBJ_FLAG_HIDDEN);
}

static int lastAppliedHeroMode = -1;

// Applied once per mode change: which effect groups exist on screen, and
// their static opacities. The per-tick animator below only moves objects
// that are actually visible.
static void applyHeroEffectVisibility() {
  bool showSun = heroMode == WeatherVisuals::VIS_SUN ||
                 heroMode == WeatherVisuals::VIS_PARTLY_CLOUDY;
  bool showCloud = heroMode == WeatherVisuals::VIS_CLOUD ||
                   heroMode == WeatherVisuals::VIS_RAIN ||
                   heroMode == WeatherVisuals::VIS_THUNDER ||
                   heroMode == WeatherVisuals::VIS_SNOW ||
                   heroMode == WeatherVisuals::VIS_PARTLY_CLOUDY;
  bool showRain = heroMode == WeatherVisuals::VIS_RAIN;
  bool showSnow = heroMode == WeatherVisuals::VIS_SNOW;
  bool showFog = heroMode == WeatherVisuals::VIS_FOG;
  bool showThunder = heroMode == WeatherVisuals::VIS_THUNDER;
  bool showNight = heroMode == WeatherVisuals::VIS_NIGHT;
  bool showBirds = heroMode == WeatherVisuals::VIS_SUN ||
                   heroMode == WeatherVisuals::VIS_PARTLY_CLOUDY;

  setEffectVisible(heroSunGlow, showSun);
  if (heroSunGlow && showSun)
    lv_obj_set_style_image_opa(heroSunGlow,
                               heroMode == WeatherVisuals::VIS_PARTLY_CLOUDY
                                   ? (lv_opa_t)170
                                   : (lv_opa_t)LV_OPA_COVER,
                               0);

  setEffectVisible(heroMoonGlow, showNight);

  // Sprites have their alpha baked in — visibility toggles are all that's
  // needed here; per-tick twinkle/fade uses image_opa below.
  for (int i = 0; i < HERO_CLOUD_NEAR + HERO_CLOUD_FAR; i++)
    setEffectVisible(heroCloudAnim[i], showCloud);
  for (int i = 0; i < HERO_RAIN_COUNT; i++)
    setEffectVisible(heroRainAnim[i], showRain);
  for (int i = 0; i < HERO_SNOW_COUNT; i++)
    setEffectVisible(heroSnowAnim[i], showSnow);
  for (int i = 0; i < HERO_FOG_COUNT; i++)
    setEffectVisible(heroFogAnim[i], showFog);
  for (int i = 0; i < HERO_STAR_COUNT; i++)
    setEffectVisible(heroStarAnim[i], showNight);
  for (int i = 0; i < HERO_BIRD_COUNT; i++)
    setEffectVisible(heroBirdAnim[i], showBirds);

  setEffectVisible(heroFlashOverlay, showThunder);
  setEffectVisible(heroBolt, showThunder);
  if (heroBolt && showThunder)
    lv_obj_set_style_image_opa(heroBolt, LV_OPA_TRANSP, 0);
  if (!showThunder) {
    nextThunderFlashMs = 0;
    thunderFlashStartMs = 0;
    thunderFlashEndMs = 0;
  }
  if (!showNight) {
    shootActive = false;
    setEffectVisible(heroShoot, false);
  }
}

static void updateHeroAnimation() {
  // Don't animate while modal overlays are open — avoids render overload
  if (settingsModal)
    return;

  uint32_t now = millis();
  if (now - lastAnimMs < 80)
    return;
  lastAnimMs = now;

  if ((int)heroMode != lastAppliedHeroMode) {
    applyHeroEffectVisibility();
    lastAppliedHeroMode = (int)heroMode;
  }

  bool showSun = heroMode == WeatherVisuals::VIS_SUN ||
                 heroMode == WeatherVisuals::VIS_PARTLY_CLOUDY;
  bool showCloud = heroMode == WeatherVisuals::VIS_CLOUD ||
                   heroMode == WeatherVisuals::VIS_RAIN ||
                   heroMode == WeatherVisuals::VIS_THUNDER ||
                   heroMode == WeatherVisuals::VIS_SNOW ||
                   heroMode == WeatherVisuals::VIS_PARTLY_CLOUDY;
  bool showRain = heroMode == WeatherVisuals::VIS_RAIN;
  bool showSnow = heroMode == WeatherVisuals::VIS_SNOW;
  bool showFog = heroMode == WeatherVisuals::VIS_FOG;
  bool showThunder = heroMode == WeatherVisuals::VIS_THUNDER;
  bool showNight = heroMode == WeatherVisuals::VIS_NIGHT;
  bool showBirds = heroMode == WeatherVisuals::VIS_SUN ||
                   heroMode == WeatherVisuals::VIS_PARTLY_CLOUDY;

  if (showSun && heroSunGlow) {
    // Gentle breathing via image scale (256 = 1.0x), pivot at sprite center
    int scale = 256 + (int)(22.0f * sinf((float)now * 0.006f));
    lv_image_set_scale(heroSunGlow, scale);
  }

  if (showNight && heroMoonGlow) {
    int scale = 256 + (int)(14.0f * sinf((float)now * 0.005f));
    lv_image_set_scale(heroMoonGlow, scale);
  }

  if (showCloud) {
    // Near layer: bigger, faster. Far layer: smaller, faded, ~half speed —
    // that speed split is what reads as depth.
    for (int i = 0; i < HERO_CLOUD_NEAR; i++) {
      if (!heroCloudAnim[i])
        continue;
      int x = ((int)(now * animWindFactor / 16.0f) + i * 270) %
                  (SCREEN_WIDTH + 340) -
              170;
      int y = 118 + i * 32;
      if (heroMode == WeatherVisuals::VIS_THUNDER)
        y += 6;
      if (heroMode == WeatherVisuals::VIS_PARTLY_CLOUDY)
        y -= 6;
      lv_obj_set_pos(heroCloudAnim[i], x, y);
    }
    for (int i = 0; i < HERO_CLOUD_FAR; i++) {
      int idx = HERO_CLOUD_NEAR + i;
      if (!heroCloudAnim[idx])
        continue;
      int x = ((int)(now * animWindFactor / 34.0f) + i * 430) %
                  (SCREEN_WIDTH + 300) -
              150;
      int y = 58 + i * 26;
      lv_obj_set_pos(heroCloudAnim[idx], x, y);
    }
  }

  if (showRain) {
    // Alternate halves each tick: keeps per-frame invalidated-area count
    // (and LVGL's per-area overhead) at half of what full motion costs.
    static bool rainAlt = false;
    rainAlt = !rainAlt;
    for (int i = 0; i < HERO_RAIN_COUNT; i++) {
      setEffectVisible(heroRainAnim[i], i < animRainCount);
      if (!heroRainAnim[i] || i >= animRainCount)
        continue;
      if (((i % 2) == 0) == rainAlt)
        continue;
      int x = (i * 53 + (int)(now * animWindFactor / 7.0f)) % SCREEN_WIDTH;
      int y = ((i * 29 + (int)(now / animRainSpeedDiv)) % 480) - 45;
      lv_obj_set_pos(heroRainAnim[i], x, y);
    }
  }

  if (showSnow) {
    static bool snowAlt = false;
    snowAlt = !snowAlt;
    for (int i = 0; i < HERO_SNOW_COUNT; i++) {
      setEffectVisible(heroSnowAnim[i], i < animSnowCount);
      if (!heroSnowAnim[i] || i >= animSnowCount)
        continue;
      if (((i % 2) == 0) == snowAlt)
        continue;
      int sway = (int)(10.0f * sinf((float)now * 0.0027f + i * 0.65f));
      int x = 20 + (i * 41) % (SCREEN_WIDTH - 40) + sway;
      int y = ((i * 19 + (int)(now / 10)) % 480) - 24;
      lv_obj_set_pos(heroSnowAnim[i], x, y);
    }
  }

  if (showFog) {
    for (int i = 0; i < HERO_FOG_COUNT; i++) {
      if (!heroFogAnim[i])
        continue;
      int wob = (int)(36.0f * (0.5f + 0.5f * sinf((float)now * 0.0012f +
                                                   i * 0.9f)));
      int opa = (int)animFogOpa - 30 + i * 20 + wob;
      lv_obj_set_style_image_opa(heroFogAnim[i],
                                 (uint8_t)constrain(opa, 40, 235), 0);
      const int width = 320;
      int x = ((int)(now / (22 + i * 8)) + i * 170) %
                  (SCREEN_WIDTH + width + 120) -
              width - 60;
      int y = 64 + i * 74;
      lv_obj_set_pos(heroFogAnim[i], x, y);
    }
  }

  if (showNight) {
    // Twinkle in three rotating groups: only a third of the stars change
    // opacity per tick. LVGL pays a fixed cost per invalidated area, so
    // batching keeps the star field alive without flooding the renderer.
    static uint8_t starPhase = 0;
    starPhase = (uint8_t)((starPhase + 1) % 3);
    for (int i = 0; i < HERO_STAR_COUNT; i++) {
      if (!heroStarAnim[i] || (i % 3) != starPhase)
        continue;
      float rate = 0.0028f + (float)(i % 4) * 0.0009f;
      uint8_t opa = (uint8_t)(70 + (int)(110.0f * (0.5f + 0.5f *
                                                    sinf((float)now * rate +
                                                         (float)i))));
      lv_obj_set_style_image_opa(heroStarAnim[i], opa, 0);
    }
  }

  if (showBirds) {
    int base = ((int)(now * (0.6f + animWindFactor * 0.5f) / 46.0f) %
                (SCREEN_WIDTH + 320)) -
               160;
    for (int i = 0; i < HERO_BIRD_COUNT; i++) {
      if (!heroBirdAnim[i])
        continue;
      int side = (i == 0) ? 0 : ((i % 2) ? 1 : -1);
      int rank = (i + 1) / 2;
      int dx = side * (10 + rank * 15);
      int dy = rank * 9;
      int bob = (int)(2.5f * sinf((float)now * 0.0021f + (float)i * 1.1f));
      lv_obj_set_pos(heroBirdAnim[i], base + dx, 96 + dy + bob);
    }
  }

  if (showNight) {
    const uint32_t SHOOT_MS = 780;
    if (shootActive) {
      uint32_t t = now - shootStartMs;
      if (t >= SHOOT_MS) {
        shootActive = false;
        setEffectVisible(heroShoot, false);
        shootRng = shootRng * 1664525u + 1013904223u;
        shootNextMs = now + 6000 + (shootRng >> 18) % 14000;
      } else {
        float f = (float)t / (float)SHOOT_MS;
        lv_obj_set_pos(heroShoot, shootX0 + (int)(250.0f * f),
                       shootY0 + (int)(110.0f * f));
        float fadeIn = f < 0.18f ? (f / 0.18f) : 1.0f;
        lv_obj_set_style_image_opa(
            heroShoot, (uint8_t)(255.0f * fadeIn * (1.0f - f * f)), 0);
      }
    } else if (now >= shootNextMs) {
      shootRng = shootRng * 1664525u + 1013904223u;
      shootX0 = (int16_t)(shootRng % 420);
      shootY0 = (int16_t)(30 + (shootRng >> 9) % 130);
      shootStartMs = now;
      shootActive = true;
      setEffectVisible(heroShoot, true);
    }
  }

  if (showThunder) {
    uint8_t flashOpa = LV_OPA_TRANSP;
    if (nextThunderFlashMs == 0)
      nextThunderFlashMs = now + 400;
    if (now >= nextThunderFlashMs && now >= thunderFlashEndMs) {
      thunderFlashStartMs = now;
      thunderFlashEndMs = now + 180;
      nextThunderFlashMs = now + 1800 + ((now / 29) % 2600);
    }
    if (now < thunderFlashEndMs) {
      uint32_t phase = now - thunderFlashStartMs;
      if (phase < 28)
        flashOpa = 110;
      else if (phase < 56)
        flashOpa = 24;
      else if (phase < 92)
        flashOpa = 150;
      else if (phase < 128)
        flashOpa = 56;
    }

    static uint8_t lastFlashOpa = 255;
    if (flashOpa != lastFlashOpa) {
      lastFlashOpa = flashOpa;
      if (heroFlashOverlay)
        lv_obj_set_style_bg_opa(heroFlashOverlay, flashOpa, 0);
      if (heroBolt)
        lv_obj_set_style_image_opa(
            heroBolt,
            flashOpa > 0 ? (uint8_t)min(255, flashOpa + 105) : LV_OPA_TRANSP,
            0);
    }
    if (flashOpa > 0 && heroBolt) {
      int jitter = ((int)(now / 12) % 2 == 0) ? 2 : -2;
      lv_obj_set_pos(heroBolt, 606 + jitter, 34);
    }
  }
}

static void updateHero() {
  if (!lblTemp)
    return;

  char buf[32];
  snprintf(buf, sizeof(buf), "%.0f\xC2\xB0%s", curCache.temp,
           tempSuffix(settingsCache.tempUnit));
  lv_label_set_text(lblTemp, buf);
  lv_label_set_text(lblCondition, curCache.conditionStr);

  char mBuf[96];
  snprintf(mBuf, sizeof(mBuf), "%s   %s   %s", curCache.humidityStr,
           curCache.windStr, curCache.precipStr);
  lv_label_set_text(lblMetrics, mBuf);

  WeatherVisuals::VisualMode mode = WeatherVisuals::fromCondition(
      curCache.conditionStr, curCache.isDay == 1, curCache.precip);
  setHeroMode(mode);

  // Motion intensity follows the real weather: clouds drift with wind,
  // rain/snow density and fall speed with precipitation, fog with humidity.
  float windMph = curCache.wind;
  if (settingsCache.windUnit == WIND_KMH)
    windMph /= 1.60934f;
  animWindFactor = constrain(0.55f + windMph / 22.0f, 0.45f, 3.2f);

  float precipMm = curCache.precip;
  if (settingsCache.precipUnit == PRECIP_IN)
    precipMm *= 25.4f;
  if (precipMm < 0.2f) {
    animRainCount = 7;
    animRainSpeedDiv = 4;
    animSnowCount = 10;
  } else if (precipMm < 1.0f) {
    animRainCount = 14;
    animRainSpeedDiv = 3;
    animSnowCount = 18;
  } else if (precipMm < 3.0f) {
    animRainCount = 21;
    animRainSpeedDiv = 3;
    animSnowCount = 24;
  } else {
    animRainCount = HERO_RAIN_COUNT;
    animRainSpeedDiv = 2;
    animSnowCount = HERO_SNOW_COUNT;
  }
  animFogOpa =
      (uint8_t)constrain(90 + (int)(curCache.humidity * 1.3f), 90, 220);

  bool brightBg = (mode == WeatherVisuals::VIS_SUN ||
                   mode == WeatherVisuals::VIS_PARTLY_CLOUDY);
  lv_color_t textCol = lv_color_hex(brightBg ? 0x0B1B33 : 0xF2F6FB);
  if (lblTime)
    lv_obj_set_style_text_color(lblTime, textCol, 0);
  if (lblAmPm)
    lv_obj_set_style_text_color(lblAmPm, textCol, 0);
  if (lblDate)
    lv_obj_set_style_text_color(lblDate, textCol, 0);
  if (lblCondition)
    lv_obj_set_style_text_color(lblCondition, textCol, 0);
  if (lblMetrics)
    lv_obj_set_style_text_color(lblMetrics, textCol, 0);
  if (lblPageHint)
    lv_obj_set_style_text_color(lblPageHint, textCol, 0);

  if (dailyCount > 0) {
    if (lblSunrise) {
      char sbuf[24];
      const char *ris = dailyCache[0].sunriseStr;
      if (ris[0] == '0')
        ris++;
      snprintf(sbuf, sizeof(sbuf), "Rise %s", ris);
      lv_label_set_text(lblSunrise, sbuf);
    }
    if (lblSunset) {
      char sbuf[24];
      const char *set = dailyCache[0].sunsetStr;
      if (set[0] == '0')
        set++;
      snprintf(sbuf, sizeof(sbuf), "Set %s", set);
      lv_label_set_text(lblSunset, sbuf);
    }
  }
}

static void updateHourly() {
  time_t now = time(nullptr);
  int32_t cutoff = 0;
  if (now > 1700000000) {
    cutoff = (int32_t)now;
  } else if (curCache.observedAt > 0) {
    cutoff = curCache.observedAt;
  } else if (hourlyCount > 0) {
    cutoff = hourlyCache[0].time;
  }
  if (cutoff > 0) {
    cutoff -= (cutoff % 3600); // show from current hour onward
  }
  size_t displayIdx = 0;

  for (size_t i = 0; i < hourlyCount && displayIdx < HOURLY_SLOTS; i++) {
    if (hourlyCache[i].time < cutoff)
      continue;

    if (hourlyCards[displayIdx])
      lv_obj_remove_flag(hourlyCards[displayIdx], LV_OBJ_FLAG_HIDDEN);
    if (hourlyTime[displayIdx]) {
      if (displayIdx == 0)
        lv_label_set_text(hourlyTime[displayIdx], "Now");
      else
        lv_label_set_text(hourlyTime[displayIdx], hourlyCache[i].timeStr);
    }
    if (hourlyIcon[displayIdx]) {
      WeatherVisuals::VisualMode mode =
          dailyModeForHour(hourlyCache[i].time, hourlyCache[i].conditionStr,
                           hourlyCache[i].precip);
      const lv_image_dsc_t *icon = WeatherVisuals::icon(mode);
      if (icon)
        lv_image_set_src(hourlyIcon[displayIdx], icon);
    }
    if (hourlyTemp[displayIdx]) {
      char tbuf[12];
      snprintf(tbuf, sizeof(tbuf), "%s\xC2\xB0", hourlyCache[i].tempStr);
      lv_label_set_text(hourlyTemp[displayIdx], tbuf);
    }
    displayIdx++;
  }

  for (size_t i = displayIdx; i < HOURLY_SLOTS; i++) {
    if (hourlyCards[i])
      lv_obj_add_flag(hourlyCards[i], LV_OBJ_FLAG_HIDDEN);
  }
}

static void updateDaily() {
  for (size_t i = 0; i < dailyCount && i < 7; i++) {
    if (dailyCards[i])
      lv_obj_remove_flag(dailyCards[i], LV_OBJ_FLAG_HIDDEN);
    if (dailyDay[i])
      lv_label_set_text(dailyDay[i], dailyCache[i].dayStr);
    if (dailyIcon[i]) {
      WeatherVisuals::VisualMode mode = WeatherVisuals::fromCondition(
          dailyCache[i].conditionStr, true, dailyCache[i].precipSum);
      const lv_image_dsc_t *icon = WeatherVisuals::icon(mode);
      if (icon)
        lv_image_set_src(dailyIcon[i], icon);
    }
    if (dailyTemp[i])
      lv_label_set_text(dailyTemp[i], dailyCache[i].tempRangeStr);
    if (dailyPrecip[i])
      lv_label_set_text(dailyPrecip[i], dailyCache[i].precipStr);
  }
  for (size_t i = dailyCount; i < 7; i++) {
    if (dailyCards[i])
      lv_obj_add_flag(dailyCards[i], LV_OBJ_FLAG_HIDDEN);
  }
}

static void updateTrend() {
  if (!trendChart || !trendTempSeries || !trendPrecipSeries)
    return;

  time_t now = time(nullptr);
  int32_t cutoff = 0;
  if (now > 1700000000) {
    cutoff = (int32_t)now;
  } else if (curCache.observedAt > 0) {
    cutoff = curCache.observedAt;
  } else if (hourlyCount > 0) {
    cutoff = hourlyCache[0].time;
  }
  if (cutoff > 0)
    cutoff -= (cutoff % 3600);

  static int16_t tempPts[24];
  static int16_t precipPts[24];
  for (int i = 0; i < 24; i++) {
    tempPts[i] = LV_CHART_POINT_NONE;
    precipPts[i] = LV_CHART_POINT_NONE;
  }

  float minTemp = 10000.0f;
  float maxTemp = -10000.0f;
  float maxPrecipProb = 0.0f;
  size_t shown = 0;
  for (size_t i = 0; i < hourlyCount && shown < 24; i++) {
    if (hourlyCache[i].time < cutoff)
      continue;
    float t = hourlyCache[i].temp;
    float p = hourlyCache[i].precipProb;
    tempPts[shown] = (int16_t)lroundf(t);
    precipPts[shown] = (int16_t)lroundf(p);
    if (t < minTemp)
      minTemp = t;
    if (t > maxTemp)
      maxTemp = t;
    if (p > maxPrecipProb)
      maxPrecipProb = p;
    shown++;
  }

  if (shown == 0) {
    minTemp = 0;
    maxTemp = 100;
  } else if (fabsf(maxTemp - minTemp) < 4.0f) {
    minTemp -= 2.0f;
    maxTemp += 2.0f;
  }

  lv_chart_set_range(trendChart, LV_CHART_AXIS_PRIMARY_Y,
                     (int16_t)floorf(minTemp - 3.0f),
                     (int16_t)ceilf(maxTemp + 3.0f));
  lv_chart_set_range(trendChart, LV_CHART_AXIS_SECONDARY_Y, 0, 100);

  for (int i = 0; i < 24; i++) {
    lv_chart_set_value_by_id(trendChart, trendTempSeries, (uint32_t)i,
                             tempPts[i]);
    lv_chart_set_value_by_id(trendChart, trendPrecipSeries, (uint32_t)i,
                             precipPts[i]);
  }
  lv_chart_refresh(trendChart);

  if (lblTrendSummary) {
    char trendBuf[96];
    if (shown == 0) {
      snprintf(trendBuf, sizeof(trendBuf), "Trend unavailable");
    } else {
      snprintf(trendBuf, sizeof(trendBuf),
               "24h: %.0f-%.0f%s | Max rain: %.0f%%", minTemp, maxTemp,
               tempSuffix(settingsCache.tempUnit), maxPrecipProb);
    }
    lv_label_set_text(lblTrendSummary, trendBuf);
  }
}

static void updateStatus() {
  // Rebuilding strings every GUI pass wastes heap and time; 1 Hz is plenty.
  static uint32_t lastBuildMs = 0;
  uint32_t now = millis();
  if (now - lastBuildMs < 1000)
    return;
  lastBuildMs = now;

  NetworkState net = Data.getNetwork();
  char buf[64];
  if (lblChipWifi) {
    if (net.connected)
      snprintf(buf, sizeof(buf), LV_SYMBOL_WIFI " %ddBm", net.rssi);
    else
      snprintf(buf, sizeof(buf), LV_SYMBOL_WIFI " Offline");
    lv_label_set_text(lblChipWifi, buf);
  }
  if (lblChipUpdated) {
    snprintf(buf, sizeof(buf), LV_SYMBOL_REFRESH " %s",
             curCache.valid ? curCache.updatedStr : "--");
    lv_label_set_text(lblChipUpdated, buf);
  }
#if ENABLE_BME280
  if (lblChipIndoor) {
    if (indoorCache.valid)
      snprintf(buf, sizeof(buf), LV_SYMBOL_HOME " %s %s", indoorCache.tempStr,
               indoorCache.humidityStr);
    else
      snprintf(buf, sizeof(buf), LV_SYMBOL_HOME " --");
    lv_label_set_text(lblChipIndoor, buf);
  }
#endif
}

static lv_obj_t *buildDots(lv_obj_t *parent, lv_obj_t **dots,
                           bool onForecastPage) {
  lv_obj_t *row = lv_obj_create(parent);
  lv_obj_set_size(row, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
  lv_obj_set_flex_flow(row, LV_FLEX_FLOW_ROW);
  lv_obj_set_style_bg_opa(row, LV_OPA_TRANSP, 0);
  lv_obj_set_style_border_width(row, 0, 0);
  lv_obj_set_style_pad_all(row, 0, 0);
  lv_obj_set_style_pad_column(row, 10, 0);
  lv_obj_clear_flag(row, LV_OBJ_FLAG_SCROLLABLE);

  for (int i = 0; i < 2; i++) {
    dots[i] = lv_obj_create(row);
    lv_obj_set_size(dots[i], 8, 8);
    lv_obj_set_style_radius(dots[i], 4, 0);
    lv_obj_set_style_bg_color(dots[i], lv_color_white(), 0);
    lv_obj_set_style_bg_opa(dots[i], i == (onForecastPage ? 1 : 0) ? LV_OPA_90
                                                                   : LV_OPA_30,
                            0);
    lv_obj_set_style_border_width(dots[i], 0, 0);
    lv_obj_clear_flag(dots[i], LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_add_flag(dots[i], LV_OBJ_FLAG_CLICKABLE);
    lv_obj_set_ext_click_area(dots[i], 10);
    lv_obj_add_event_cb(dots[i], onDotClicked, LV_EVENT_CLICKED,
                        (void *)(i == 1));
  }
  return row;
}

void build() {
  Theme::init();
  WebConfig::logPrintf("[UI] build start\n");
  WeatherVisuals::init();

  // ---- home screen: full-bleed sky ------------------------------------
  homeScreen = lv_obj_create(nullptr);
  lv_obj_set_style_bg_color(homeScreen, Theme::COLOR_NIGHT_TOP, 0);
  lv_obj_clear_flag(homeScreen, LV_OBJ_FLAG_SCROLLABLE);
  lv_obj_add_event_cb(homeScreen, onHomeGesture, LV_EVENT_GESTURE, nullptr);

  heroBgImage = lv_image_create(homeScreen);
  lv_obj_set_size(heroBgImage, SCREEN_WIDTH, SCREEN_HEIGHT);
  lv_obj_set_pos(heroBgImage, 0, 0);
  lv_obj_clear_flag(heroBgImage, LV_OBJ_FLAG_SCROLLABLE);
  const lv_image_dsc_t *bg = WeatherVisuals::background(heroMode);
  if (bg)
    lv_image_set_src(heroBgImage, bg);

  // Hero effect sprites (procedurally rendered ARGB art from WeatherVisuals)
  auto makeSprite = [](lv_obj_t *parent, WeatherVisuals::SpriteId id) {
    lv_obj_t *img = lv_image_create(parent);
    const lv_image_dsc_t *src = WeatherVisuals::sprite(id);
    if (src)
      lv_image_set_src(img, src);
    lv_obj_add_flag(img, LV_OBJ_FLAG_IGNORE_LAYOUT);
    lv_obj_clear_flag(img, LV_OBJ_FLAG_SCROLLABLE);
    return img;
  };

  heroSunGlow = makeSprite(homeScreen, WeatherVisuals::SPR_SUN);
  lv_obj_set_pos(heroSunGlow, 638, 18);
  lv_image_set_pivot(heroSunGlow, 60, 60);

  heroMoonGlow = makeSprite(homeScreen, WeatherVisuals::SPR_MOON);
  lv_obj_set_pos(heroMoonGlow, 648, 26);
  lv_image_set_pivot(heroMoonGlow, 50, 50);

  for (int i = 0; i < HERO_CLOUD_FAR; i++) {
    int idx = HERO_CLOUD_NEAR + i;
    heroCloudAnim[idx] = makeSprite(homeScreen, WeatherVisuals::SPR_CLOUD_FAR);
    lv_obj_set_pos(heroCloudAnim[idx], i * 420 - 120, 58 + i * 26);
    lv_obj_set_style_image_opa(heroCloudAnim[idx], 165, 0);
  }

  for (int i = 0; i < HERO_CLOUD_NEAR; i++) {
    heroCloudAnim[i] = makeSprite(
        homeScreen,
        (i == 1) ? WeatherVisuals::SPR_CLOUD_BIG : WeatherVisuals::SPR_CLOUD);
    lv_obj_set_pos(heroCloudAnim[i], i * 250 - 80, 118 + i * 32);
  }

  for (int i = 0; i < HERO_RAIN_COUNT; i++) {
    heroRainAnim[i] = makeSprite(homeScreen, WeatherVisuals::SPR_RAIN);
    lv_obj_set_pos(heroRainAnim[i], (i * 53) % SCREEN_WIDTH, 90);
  }

  for (int i = 0; i < HERO_SNOW_COUNT; i++) {
    heroSnowAnim[i] = makeSprite(
        homeScreen, (i % 3 == 0) ? WeatherVisuals::SPR_SNOW_S
                                 : ((i % 3 == 1) ? WeatherVisuals::SPR_SNOW
                                                 : WeatherVisuals::SPR_SNOW_L));
    lv_obj_set_pos(heroSnowAnim[i], 24 + (i * 41) % 740, 96 + i * 7);
  }

  for (int i = 0; i < HERO_BIRD_COUNT; i++) {
    heroBirdAnim[i] = makeSprite(homeScreen, WeatherVisuals::SPR_BIRD);
    lv_obj_set_pos(heroBirdAnim[i], -40 + i * 18, 100 + i * 8);
  }

  heroShoot = makeSprite(homeScreen, WeatherVisuals::SPR_SHOOT);
  lv_obj_set_pos(heroShoot, -60, 40);
  lv_obj_set_style_image_opa(heroShoot, LV_OPA_TRANSP, 0);
  setEffectVisible(heroShoot, false);

  for (int i = 0; i < HERO_FOG_COUNT; i++) {
    heroFogAnim[i] = makeSprite(homeScreen, WeatherVisuals::SPR_FOG);
    lv_obj_set_pos(heroFogAnim[i], i * 140 - 180, 64 + i * 74);
  }

  heroFlashOverlay = lv_obj_create(homeScreen);
  lv_obj_set_size(heroFlashOverlay, SCREEN_WIDTH, SCREEN_HEIGHT);
  lv_obj_set_pos(heroFlashOverlay, 0, 0);
  lv_obj_set_style_bg_color(heroFlashOverlay, lv_color_hex(0xF9FBFF), 0);
  lv_obj_set_style_bg_opa(heroFlashOverlay, LV_OPA_TRANSP, 0);
  lv_obj_set_style_border_width(heroFlashOverlay, 0, 0);
  lv_obj_add_flag(heroFlashOverlay, LV_OBJ_FLAG_IGNORE_LAYOUT);
  lv_obj_clear_flag(heroFlashOverlay, LV_OBJ_FLAG_SCROLLABLE);

  heroBolt = makeSprite(homeScreen, WeatherVisuals::SPR_BOLT);
  lv_obj_set_pos(heroBolt, 628, 62);
  lv_obj_set_style_image_opa(heroBolt, LV_OPA_TRANSP, 0);

  for (int i = 0; i < HERO_STAR_COUNT; i++) {
    heroStarAnim[i] = makeSprite(
        homeScreen, (i % 3 == 0) ? WeatherVisuals::SPR_STAR_S
                                 : ((i % 3 == 1) ? WeatherVisuals::SPR_STAR
                                                 : WeatherVisuals::SPR_STAR_L));
    lv_obj_set_pos(heroStarAnim[i], 30 + (i * 63) % 700, 16 + (i * 29) % 300);
  }

  // ---- clock cluster ---------------------------------------------------
  lv_obj_t *timeRow = lv_obj_create(homeScreen);
  lv_obj_set_size(timeRow, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
  lv_obj_set_flex_flow(timeRow, LV_FLEX_FLOW_ROW);
  lv_obj_set_flex_align(timeRow, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_END,
                        LV_FLEX_ALIGN_END);
  lv_obj_set_style_bg_opa(timeRow, LV_OPA_TRANSP, 0);
  lv_obj_set_style_border_width(timeRow, 0, 0);
  lv_obj_set_style_pad_all(timeRow, 0, 0);
  lv_obj_set_style_pad_column(timeRow, 10, 0);
  lv_obj_clear_flag(timeRow, LV_OBJ_FLAG_SCROLLABLE);
  lv_obj_align(timeRow, LV_ALIGN_TOP_MID, 0, 46);

  lblTime = lv_label_create(timeRow);
  lv_obj_set_style_text_font(lblTime, &lv_font_clock_96, 0);
  lv_obj_set_style_text_color(lblTime, lv_color_hex(0xF2F6FB), 0);
  lv_label_set_text(lblTime, "--:--");

  lblAmPm = lv_label_create(timeRow);
  lv_obj_set_style_text_font(lblAmPm, &lv_font_montserrat_24, 0);
  lv_obj_set_style_text_color(lblAmPm, lv_color_hex(0xF2F6FB), 0);
  lv_obj_set_style_pad_bottom(lblAmPm, 24, 0);
  lv_label_set_text(lblAmPm, "AM");

  lblDate = lv_label_create(homeScreen);
  lv_obj_set_style_text_font(lblDate, &lv_font_montserrat_24, 0);
  lv_obj_set_style_text_color(lblDate, lv_color_hex(0xF2F6FB), 0);
  lv_label_set_text(lblDate, "--");
  lv_obj_align(lblDate, LV_ALIGN_TOP_MID, 0, 176);

  lv_obj_t *tempRow = lv_obj_create(homeScreen);
  lv_obj_set_size(tempRow, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
  lv_obj_set_flex_flow(tempRow, LV_FLEX_FLOW_ROW);
  lv_obj_set_flex_align(tempRow, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_END,
                        LV_FLEX_ALIGN_END);
  lv_obj_set_style_bg_opa(tempRow, LV_OPA_TRANSP, 0);
  lv_obj_set_style_border_width(tempRow, 0, 0);
  lv_obj_set_style_pad_all(tempRow, 0, 0);
  lv_obj_set_style_pad_column(tempRow, 14, 0);
  lv_obj_clear_flag(tempRow, LV_OBJ_FLAG_SCROLLABLE);
  lv_obj_align(tempRow, LV_ALIGN_TOP_MID, 0, 208);

  lblTemp = lv_label_create(tempRow);
  lv_obj_set_style_text_font(lblTemp, &lv_font_temp_64, 0);
  lv_obj_set_style_text_color(lblTemp, lv_color_hex(0xF2F6FB), 0);
  lv_label_set_text(lblTemp, "--");

  lblCondition = lv_label_create(tempRow);
  lv_obj_set_style_text_font(lblCondition, &lv_font_montserrat_24, 0);
  lv_obj_set_style_text_color(lblCondition, lv_color_hex(0xF2F6FB), 0);
  lv_obj_set_style_pad_bottom(lblCondition, 12, 0);
  lv_label_set_text(lblCondition, "Loading...");

  lblMetrics = lv_label_create(homeScreen);
  lv_obj_set_style_text_font(lblMetrics, &lv_font_montserrat_16, 0);
  lv_obj_set_style_text_color(lblMetrics, lv_color_hex(0xF2F6FB), 0);
  lv_label_set_text(lblMetrics, "--");
  lv_obj_align(lblMetrics, LV_ALIGN_TOP_MID, 0, 286);

  lblPageHint = lv_label_create(homeScreen);
  lv_label_set_text(lblPageHint, LV_SYMBOL_UP "  Forecast");
  lv_obj_set_style_text_font(lblPageHint, &lv_font_montserrat_16, 0);
  lv_obj_set_style_text_color(lblPageHint, lv_color_hex(0xF2F6FB), 0);
  lv_obj_set_style_text_opa(lblPageHint, LV_OPA_60, 0);
  lv_obj_align(lblPageHint, LV_ALIGN_TOP_MID, 0, 310);

  lv_obj_align(buildDots(homeScreen, dotHome, false), LV_ALIGN_TOP_MID, 0, 334);

  // ---- status chips (top-left) ----------------------------------------
  auto makeChip = [](lv_obj_t *parent, const char *txt) {
    lv_obj_t *l = lv_label_create(parent);
    lv_label_set_text(l, txt);
    lv_obj_set_style_text_font(l, &lv_font_montserrat_16, 0);
    lv_obj_set_style_text_color(l, lv_color_hex(0xFFFFFF), 0);
    lv_obj_set_style_bg_color(l, lv_color_black(), 0);
    lv_obj_set_style_bg_opa(l, LV_OPA_40, 0);
    lv_obj_set_style_radius(l, 13, 0);
    lv_obj_set_style_pad_hor(l, 10, 0);
    lv_obj_set_style_pad_ver(l, 6, 0);
    return l;
  };

  lv_obj_t *chipRow = lv_obj_create(homeScreen);
  lv_obj_set_size(chipRow, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
  lv_obj_set_flex_flow(chipRow, LV_FLEX_FLOW_ROW);
  lv_obj_set_style_bg_opa(chipRow, LV_OPA_TRANSP, 0);
  lv_obj_set_style_border_width(chipRow, 0, 0);
  lv_obj_set_style_pad_all(chipRow, 0, 0);
  lv_obj_set_style_pad_column(chipRow, 8, 0);
  lv_obj_clear_flag(chipRow, LV_OBJ_FLAG_SCROLLABLE);
  lv_obj_align(chipRow, LV_ALIGN_TOP_LEFT, 10, 10);

  lblChipWifi = makeChip(chipRow, LV_SYMBOL_WIFI " --");
  lblChipIndoor = makeChip(chipRow, LV_SYMBOL_HOME " --");
  lblChipUpdated = makeChip(chipRow, LV_SYMBOL_REFRESH " --");

  // ---- top-right icon buttons -----------------------------------------
  auto makeIconBtn = [](lv_obj_t *parent, const char *symbol,
                        lv_event_cb_t cb) {
    lv_obj_t *b = lv_btn_create(parent);
    lv_obj_set_size(b, 44, 44);
    lv_obj_set_style_radius(b, 22, 0);
    lv_obj_set_style_bg_color(b, lv_color_black(), 0);
    lv_obj_set_style_bg_opa(b, LV_OPA_40, 0);
    lv_obj_set_style_shadow_width(b, 0, 0);
    lv_obj_add_event_cb(b, cb, LV_EVENT_CLICKED, nullptr);
    lv_obj_t *l = lv_label_create(b);
    lv_label_set_text(l, symbol);
    lv_obj_set_style_text_font(l, &lv_font_montserrat_20, 0);
    lv_obj_set_style_text_color(l, lv_color_hex(0xFFFFFF), 0);
    lv_obj_center(l);
    return b;
  };

  btnRefresh = makeIconBtn(homeScreen, LV_SYMBOL_REFRESH, onRefreshWeather);
  lv_obj_set_pos(btnRefresh, SCREEN_WIDTH - 116, 10);
  btnSettings = makeIconBtn(homeScreen, LV_SYMBOL_SETTINGS, onSettingsClick);
  lv_obj_set_pos(btnSettings, SCREEN_WIDTH - 62, 10);

  // ---- frosted hourly strip -------------------------------------------
  hourlyPanel = lv_obj_create(homeScreen);
  lv_obj_set_size(hourlyPanel, SCREEN_WIDTH - 16, 116);
  lv_obj_set_pos(hourlyPanel, 8, SCREEN_HEIGHT - 124);
  lv_obj_set_style_bg_color(hourlyPanel, lv_color_black(), 0);
  lv_obj_set_style_bg_opa(hourlyPanel, LV_OPA_40, 0);
  lv_obj_set_style_radius(hourlyPanel, 18, 0);
  lv_obj_set_style_border_color(hourlyPanel, lv_color_white(), 0);
  lv_obj_set_style_border_opa(hourlyPanel, LV_OPA_20, 0);
  lv_obj_set_style_border_width(hourlyPanel, 1, 0);
  lv_obj_set_style_pad_all(hourlyPanel, 8, 0);
  lv_obj_set_style_pad_column(hourlyPanel, 6, 0);
  lv_obj_set_flex_flow(hourlyPanel, LV_FLEX_FLOW_ROW);
  lv_obj_clear_flag(hourlyPanel, LV_OBJ_FLAG_SCROLLABLE);

  for (int i = 0; i < HOURLY_SLOTS; i++) {
    hourlyCards[i] = lv_obj_create(hourlyPanel);
    lv_obj_set_height(hourlyCards[i], LV_PCT(100));
    lv_obj_set_flex_grow(hourlyCards[i], 1);
    lv_obj_set_flex_flow(hourlyCards[i], LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(hourlyCards[i], LV_FLEX_ALIGN_SPACE_EVENLY,
                          LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_set_style_bg_opa(hourlyCards[i], LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_width(hourlyCards[i], 0, 0);
    lv_obj_set_style_pad_all(hourlyCards[i], 0, 0);
    lv_obj_clear_flag(hourlyCards[i], LV_OBJ_FLAG_SCROLLABLE);

    hourlyTime[i] = lv_label_create(hourlyCards[i]);
    lv_obj_set_style_text_font(hourlyTime[i], &lv_font_montserrat_16, 0);
    lv_obj_set_style_text_color(hourlyTime[i], lv_color_hex(0xDCE6F2), 0);
    lv_label_set_text(hourlyTime[i], "--");

    hourlyIcon[i] = lv_image_create(hourlyCards[i]);
    const lv_image_dsc_t *hic = WeatherVisuals::icon(WeatherVisuals::VIS_CLOUD);
    if (hic)
      lv_image_set_src(hourlyIcon[i], hic);
    lv_obj_set_size(hourlyIcon[i], 40, 40);
    lv_image_set_scale(hourlyIcon[i], 160);

    hourlyTemp[i] = lv_label_create(hourlyCards[i]);
    lv_obj_set_style_text_font(hourlyTemp[i], &lv_font_montserrat_20, 0);
    lv_obj_set_style_text_color(hourlyTemp[i], lv_color_hex(0xFFFFFF), 0);
    lv_label_set_text(hourlyTemp[i], "--");
  }

  // ---- toast (refresh feedback) ---------------------------------------
  toastLabel = lv_label_create(homeScreen);
  lv_label_set_text(toastLabel, "");
  lv_obj_add_flag(toastLabel, LV_OBJ_FLAG_HIDDEN);
  lv_obj_set_style_text_font(toastLabel, &lv_font_montserrat_16, 0);
  lv_obj_set_style_text_color(toastLabel, Theme::COLOR_TEXT, 0);
  lv_obj_set_style_bg_color(toastLabel, Theme::COLOR_PANEL, 0);
  lv_obj_set_style_bg_opa(toastLabel, LV_OPA_90, 0);
  lv_obj_set_style_border_color(toastLabel, Theme::COLOR_BORDER, 0);
  lv_obj_set_style_border_width(toastLabel, 1, 0);
  lv_obj_set_style_radius(toastLabel, 14, 0);
  lv_obj_set_style_pad_hor(toastLabel, 14, 0);
  lv_obj_set_style_pad_ver(toastLabel, 7, 0);
  lv_obj_align(toastLabel, LV_ALIGN_TOP_MID, 0, 10);

  WebConfig::logPrintf("[UI] home built\n");

  // ---- forecast screen -------------------------------------------------
  forecastScreen = lv_obj_create(nullptr);
  lv_obj_set_style_bg_color(forecastScreen, lv_color_hex(0x0C1220), 0);
  lv_obj_set_style_bg_grad_color(forecastScreen, lv_color_hex(0x131C2E), 0);
  lv_obj_set_style_bg_grad_dir(forecastScreen, LV_GRAD_DIR_VER, 0);
  lv_obj_clear_flag(forecastScreen, LV_OBJ_FLAG_SCROLLABLE);
  lv_obj_add_event_cb(forecastScreen, onForecastGesture, LV_EVENT_GESTURE,
                      nullptr);

  lv_obj_t *fTitle = lv_label_create(forecastScreen);
  lv_label_set_text(fTitle, "Forecast");
  lv_obj_set_style_text_font(fTitle, &lv_font_montserrat_24, 0);
  lv_obj_set_style_text_color(fTitle, Theme::COLOR_TEXT, 0);
  lv_obj_set_pos(fTitle, 18, 12);

  lv_obj_t *downHint = lv_label_create(forecastScreen);
  lv_label_set_text(downHint, LV_SYMBOL_DOWN);
  lv_obj_set_style_text_font(downHint, &lv_font_montserrat_16, 0);
  lv_obj_set_style_text_color(downHint, Theme::COLOR_SUBTEXT, 0);
  lv_obj_set_style_text_opa(downHint, LV_OPA_60, 0);
  lv_obj_align(downHint, LV_ALIGN_TOP_MID, 0, 15);

  lv_obj_t *sunRow = lv_obj_create(forecastScreen);
  lv_obj_set_size(sunRow, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
  lv_obj_set_flex_flow(sunRow, LV_FLEX_FLOW_ROW);
  lv_obj_set_style_bg_opa(sunRow, LV_OPA_TRANSP, 0);
  lv_obj_set_style_border_width(sunRow, 0, 0);
  lv_obj_set_style_pad_all(sunRow, 0, 0);
  lv_obj_set_style_pad_column(sunRow, 18, 0);
  lv_obj_clear_flag(sunRow, LV_OBJ_FLAG_SCROLLABLE);
  lv_obj_align(sunRow, LV_ALIGN_TOP_RIGHT, -18, 18);

  lblSunrise = lv_label_create(sunRow);
  lv_obj_set_style_text_font(lblSunrise, &lv_font_montserrat_16, 0);
  lv_obj_set_style_text_color(lblSunrise, Theme::COLOR_SUNRISE, 0);
  lv_label_set_text(lblSunrise, "Rise --");

  lblSunset = lv_label_create(sunRow);
  lv_obj_set_style_text_font(lblSunset, &lv_font_montserrat_16, 0);
  lv_obj_set_style_text_color(lblSunset, Theme::COLOR_SUNSET, 0);
  lv_label_set_text(lblSunset, "Set --");

  trendCard = lv_obj_create(forecastScreen);
  lv_obj_set_size(trendCard, SCREEN_WIDTH - 24, 182);
  lv_obj_set_pos(trendCard, 12, 48);
  lv_obj_add_style(trendCard, &Theme::styleCard, 0);
  lv_obj_set_flex_flow(trendCard, LV_FLEX_FLOW_COLUMN);
  lv_obj_set_style_pad_row(trendCard, 4, 0);
  lv_obj_clear_flag(trendCard, LV_OBJ_FLAG_SCROLLABLE);

  lv_obj_t *trendLabel = lv_label_create(trendCard);
  lv_label_set_text(trendLabel, "Next 24 Hours");
  lv_obj_set_style_text_font(trendLabel, &lv_font_montserrat_20, 0);
  lv_obj_set_style_text_color(trendLabel, Theme::COLOR_TEXT, 0);

  trendChart = lv_chart_create(trendCard);
  lv_obj_set_size(trendChart, LV_PCT(100), 108);
  lv_chart_set_type(trendChart, LV_CHART_TYPE_LINE);
  lv_chart_set_update_mode(trendChart, LV_CHART_UPDATE_MODE_SHIFT);
  lv_chart_set_div_line_count(trendChart, 4, 6);
  lv_chart_set_point_count(trendChart, 24);
  lv_obj_set_style_bg_opa(trendChart, LV_OPA_TRANSP, 0);
  lv_obj_set_style_border_width(trendChart, 0, 0);
  trendTempSeries = lv_chart_add_series(trendChart, Theme::COLOR_WARM,
                                        LV_CHART_AXIS_PRIMARY_Y);
  trendPrecipSeries = lv_chart_add_series(trendChart, Theme::COLOR_COOL,
                                          LV_CHART_AXIS_SECONDARY_Y);

  lblTrendSummary = lv_label_create(trendCard);
  lv_obj_set_style_text_color(lblTrendSummary, Theme::COLOR_SUBTEXT, 0);
  lv_obj_set_style_text_font(lblTrendSummary, &lv_font_montserrat_16, 0);
  lv_label_set_text(lblTrendSummary, "24h: --");

  dailyCont = lv_obj_create(forecastScreen);
  lv_obj_set_size(dailyCont, SCREEN_WIDTH - 24, 222);
  lv_obj_set_pos(dailyCont, 12, 234);
  lv_obj_set_flex_flow(dailyCont, LV_FLEX_FLOW_COLUMN);
  lv_obj_set_style_bg_opa(dailyCont, LV_OPA_TRANSP, 0);
  lv_obj_set_style_border_width(dailyCont, 0, 0);
  lv_obj_set_style_pad_all(dailyCont, 0, 0);
  lv_obj_set_style_pad_row(dailyCont, 2, 0);
  lv_obj_clear_flag(dailyCont, LV_OBJ_FLAG_SCROLLABLE);

  for (int i = 0; i < 7; i++) {
    dailyCards[i] = lv_obj_create(dailyCont);
    lv_obj_set_size(dailyCards[i], LV_PCT(100), 30);
    lv_obj_add_style(dailyCards[i], &Theme::styleCard, 0);
    lv_obj_set_style_pad_ver(dailyCards[i], 2, 0);
    lv_obj_set_style_pad_hor(dailyCards[i], 12, 0);
    lv_obj_set_flex_flow(dailyCards[i], LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(dailyCards[i], LV_FLEX_ALIGN_SPACE_BETWEEN,
                          LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_clear_flag(dailyCards[i], LV_OBJ_FLAG_SCROLLABLE);

    dailyDay[i] = lv_label_create(dailyCards[i]);
    lv_obj_set_style_text_font(dailyDay[i], &lv_font_montserrat_20, 0);
    lv_obj_set_style_text_color(dailyDay[i], Theme::COLOR_TEXT, 0);
    lv_obj_set_width(dailyDay[i], 130);
    lv_label_set_text(dailyDay[i], "--");

    dailyIcon[i] = lv_image_create(dailyCards[i]);
    const lv_image_dsc_t *dic = WeatherVisuals::icon(WeatherVisuals::VIS_SUN);
    if (dic)
      lv_image_set_src(dailyIcon[i], dic);
    lv_obj_set_size(dailyIcon[i], 26, 26);
    lv_image_set_scale(dailyIcon[i], 104);

    dailyTemp[i] = lv_label_create(dailyCards[i]);
    lv_obj_set_style_text_font(dailyTemp[i], &lv_font_montserrat_20, 0);
    lv_obj_set_style_text_color(dailyTemp[i], Theme::COLOR_TEXT, 0);
    lv_label_set_text(dailyTemp[i], "--");

    dailyPrecip[i] = lv_label_create(dailyCards[i]);
    lv_obj_set_style_text_font(dailyPrecip[i], &lv_font_montserrat_16, 0);
    lv_obj_set_style_text_color(dailyPrecip[i], Theme::COLOR_COOL, 0);
    lv_obj_set_width(dailyPrecip[i], 90);
    lv_label_set_text(dailyPrecip[i], "--");
  }

  lv_obj_align(buildDots(forecastScreen, dotForecast, true),
               LV_ALIGN_BOTTOM_MID, 0, -8);

  WebConfig::logPrintf("[UI] forecast built\n");

  lv_screen_load(homeScreen);
  updatePageDots(false);

  updateTrend();
  setHeroMode(heroMode);
  updateHeroAnimation();

  WebConfig::logPrintf("[UI] Built\n");
}

void loop() {
  bool modalOpen = (settingsModal != nullptr);

  // Retire the toast once its message has had its time.
  if (toastLabel && !lv_obj_has_flag(toastLabel, LV_OBJ_FLAG_HIDDEN) &&
      millis() > toastUntilMs)
    lv_obj_add_flag(toastLabel, LV_OBJ_FLAG_HIDDEN);

  // Only rewrite the big clock labels when the text actually changes (once
  // a second), not on every GUI pass — each set_text invalidates and forces
  // a redraw of the label area.
  NetworkState net = Data.getNetwork();
  static char lastTimeShown[8] = {0};
  static char lastAmPmShown[4] = {0};
  static char lastDateShown[24] = {0};
  if (lblTime && strcmp(net.timeShort, lastTimeShown) != 0) {
    lv_label_set_text(lblTime, net.timeShort);
    strncpy(lastTimeShown, net.timeShort, sizeof(lastTimeShown) - 1);
  }
  if (lblAmPm && strcmp(net.ampm, lastAmPmShown) != 0) {
    lv_label_set_text(lblAmPm, net.ampm);
    strncpy(lastAmPmShown, net.ampm, sizeof(lastAmPmShown) - 1);
  }
  if (lblDate && strcmp(net.dateStr, lastDateShown) != 0) {
    lv_label_set_text(lblDate, net.dateStr);
    strncpy(lastDateShown, net.dateStr, sizeof(lastDateShown) - 1);
  }

  // Defer heavy data-driven UI updates while a modal overlay is open.
  // Version numbers will still diverge, so updates apply on the next
  // frame after the modal closes.
  if (!modalOpen) {
    uint32_t weatherVer = Data.getWeatherVersion();
    if (weatherVer != lastWeatherVer) {
      curCache = Data.getCurrent();
      hourlyCount = Data.getHourly(hourlyCache, WEATHER_MAX_HOURLY);
      dailyCount = Data.getDaily(dailyCache, WEATHER_MAX_DAILY);
      lastWeatherVer = weatherVer;

      updateHero();
      updateHourly();
      updateTrend();
      updateDaily();
    }

#if ENABLE_BME280
    IndoorData in = Data.getIndoor();
    if (in.version != lastIndoorVer) {
      indoorCache = in;
      lastIndoorVer = in.version;
      updateHero();
    }
#endif
  }

  uint32_t settingsVer = Data.getSettingsVersion();
  if (settingsVer != lastSettingsVer) {
    settingsCache = Data.getSettings();
    lastSettingsVer = settingsVer;
    Backlight::set(settingsCache.brightness);
    if (!modalOpen)
      updateTrend();
  }

  uint32_t diagVer = Data.getDiagnosticsVersion();
  if (diagVer != lastDiagVer) {
    diagCache = Data.getDiagnostics();
    lastDiagVer = diagVer;
    updateDiagnosticsPanel();
  }

  updateHeroAnimation();
  updateStatus();
}

} // namespace UI
