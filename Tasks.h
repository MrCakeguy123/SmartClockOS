#pragma once

#include "Config.h"
#include "DataManager.h"
#include "ClockNetwork.h"
#include "Weather.h"
#include "Sensor.h"
#include "Fan.h"
#include "Display.h"
#include "Touch.h"
#include "UI.h"
#include "Splash.h"
#include <Arduino.h>
#include <esp_arduino_version.h>
#include <esp_heap_caps.h>
#include <esp_task_wdt.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

#if defined(configGENERATE_RUN_TIME_STATS) && configGENERATE_RUN_TIME_STATS == 1
static uint64_t lastCpuTime = 0;
static uint32_t lastIdle0 = 0;
static uint32_t lastIdle1 = 0;
#endif

static void computeSystemStats() {
    uint32_t heapSize = ESP.getHeapSize();
    uint32_t heapFree = ESP.getFreeHeap();
    uint32_t heapUsed = heapSize > heapFree ? heapSize - heapFree : 0;
    uint32_t psramSize = ESP.getPsramSize();
    uint32_t psramFree = ESP.getFreePsram();
    uint32_t psramUsed = psramSize > psramFree ? psramSize - psramFree : 0;
    
    char ramBuf[96];
    if (psramSize > 0) {
        snprintf(ramBuf, sizeof(ramBuf), "RAM: %u/%u KB | PSRAM: %u/%u KB",
                 heapUsed / 1024, heapSize / 1024, psramUsed / 1024, psramSize / 1024);
    } else {
        snprintf(ramBuf, sizeof(ramBuf), "RAM: %u/%u KB", heapUsed / 1024, heapSize / 1024);
    }
    
    char cpuBuf[48] = "CPU: --";
    
#if defined(configGENERATE_RUN_TIME_STATS) && configGENERATE_RUN_TIME_STATS == 1
    uint64_t now = esp_timer_get_time();
    if (lastCpuTime > 0) {
        TaskStatus_t idle0Info, idle1Info;
        vTaskGetInfo(xTaskGetIdleTaskHandleForCPU(0), &idle0Info, pdTRUE, eInvalid);
        vTaskGetInfo(xTaskGetIdleTaskHandleForCPU(1), &idle1Info, pdTRUE, eInvalid);
        
        uint32_t idle0Delta = idle0Info.ulRunTimeCounter - lastIdle0;
        uint32_t idle1Delta = idle1Info.ulRunTimeCounter - lastIdle1;
        uint64_t totalDelta = now - lastCpuTime;
        
        if (totalDelta > 0) {
            uint32_t usage0 = idle0Delta < totalDelta ? (uint32_t)(100 - (idle0Delta * 100ULL / totalDelta)) : 0;
            uint32_t usage1 = idle1Delta < totalDelta ? (uint32_t)(100 - (idle1Delta * 100ULL / totalDelta)) : 0;
            snprintf(cpuBuf, sizeof(cpuBuf), "CPU: C0 %u%% | C1 %u%%", usage0, usage1);
        }
        
        lastIdle0 = idle0Info.ulRunTimeCounter;
        lastIdle1 = idle1Info.ulRunTimeCounter;
    }
    lastCpuTime = now;
#endif
    
    Data.setSystemInfo(cpuBuf, ramBuf);
    Data.setDiagMemory(heapFree, heapSize, psramFree, psramSize);

    NetworkState net = Data.getNetwork();
    uint8_t quality = 0;
    if (net.connected) {
        int q = (net.rssi + 100) * 2;
        if (q < 0) q = 0;
        if (q > 100) q = 100;
        quality = (uint8_t)q;
    }
    Data.setDiagWifi(net.connected, net.rssi, quality);
}

void networkTask(void* param) {
    Serial.println("[NetTask] Starting on Core 0");
    
    Net::init();
    
    uint32_t lastWeather = 0;

    for (;;) {
        Net::loop();

        uint32_t now = millis();
        bool manualRefresh = Data.consumeWeatherRefresh();
        bool hasWeather = Data.getCurrent().valid;
        uint32_t weatherInterval = hasWeather ? WEATHER_INTERVAL_MS : 30000;

        if (manualRefresh || now - lastWeather > weatherInterval || lastWeather == 0) {
            Weather::fetch();
            lastWeather = now;
        }

        vTaskDelay(50 / portTICK_PERIOD_MS);
    }
}

void helperTask(void* param) {
    Serial.println("[HelperTask] Starting on Core 0");
    
    // Stagger helper startup to avoid I2C init races during early boot.
    vTaskDelay(2000 / portTICK_PERIOD_MS);
    
#if ENABLE_BME280
    Sensor::init();
#endif
#if ENABLE_FAN
    Fan::init();
#endif

    uint32_t lastStats = 0;
    uint32_t lastNightCheck = 0;
    uint32_t lastNightSettingsVer = 0;
#if ENABLE_FAN
    uint32_t lastFan = 0;
#endif
#if ENABLE_BME280
    uint32_t lastSensor = 0;
#endif

    for (;;) {
        uint32_t now = millis();

        if (now - lastStats >= 1000) {
            computeSystemStats();
            lastStats = now;
        }

#if ENABLE_FAN
        if (now - lastFan >= 2000) {
            Fan::loop();
            lastFan = now;
        }
#endif

        // Re-evaluate night mode as the clock crosses the start/end hours,
        // not just when settings are saved.
        if (now - lastNightCheck >= 5000) {
            Settings s = Data.getSettings();
            uint32_t ver = Data.getSettingsVersion();
            bool active = isNightModeActiveNow(s);
            if (active != Backlight::isNightMode() || ver != lastNightSettingsVer) {
                Backlight::setNightMode(active, s.nightBrightness);
                lastNightSettingsVer = ver;
            }
            lastNightCheck = now;
        }
        
#if ENABLE_BME280
        if (now - lastSensor >= BME280_INTERVAL_MS) {
            Sensor::read();
            lastSensor = now;
        }
#endif
        
        vTaskDelay(50 / portTICK_PERIOD_MS);
    }
}

void guiTask(void* param) {
    Serial.printf("[GUITask] Starting on Core %d\n", xPortGetCoreID());

    // Backlight comes up DARK and stays dark until there's a real frame to
    // show — the panel's start-up garbage is invisible instead of being the
    // first thing you see.
    Backlight::init();

    Display::init();
    if (!Display::getDisplay()) {
        Serial.println("[GUITask] Display init failed, retrying...");
        for (;;) vTaskDelay(1000 / portTICK_PERIOD_MS);
    }
    Touch::init();
    Theme::init();
    Touch::createInput(Display::getDisplay());

    // Let the web server serve /api/screenshot from the live panel buffer.
    WebConfig::setFramebufferSource(Display::rawFramebuffer, SCREEN_WIDTH,
                                    SCREEN_HEIGHT);

    // Draw the branded splash before anything heavy, render it once, and
    // only then bring the light up — so the reveal is a finished frame.
    Splash::show();
    Splash::step("Starting up", 8);

    Settings settings = Data.getSettings();
    Backlight::set(settings.brightness);
    if (isNightModeActiveNow(settings)) {
        Backlight::setNightMode(true, settings.nightBrightness);
    }
    Backlight::fadeUp(450);

    Splash::step("Loading graphics", 25);
    UI::build();
    Splash::step("Preparing interface", 70);

    // Hold the splash while the network settles so the first thing revealed
    // is populated, not a skeleton with "--" everywhere. Strictly bounded:
    // offline or a dead API just means we drop through after the timeout.
    const uint32_t NET_WAIT_MS = 9000;
    uint32_t waitStart = millis();
    bool announcedWifi = false;
    while (Splash::active() && millis() - waitStart < NET_WAIT_MS) {
        bool wifiUp = Data.getNetwork().connected;
        if (wifiUp && Data.getCurrent().valid) break;

        if (wifiUp && !announcedWifi) {
            Splash::step("Fetching weather", 85);
            announcedWifi = true;
        } else if (!wifiUp) {
            Splash::step("Connecting to Wi-Fi", 78);
        } else {
            Splash::step(nullptr, 85 + (int)((millis() - waitStart) / 900));
        }
        vTaskDelay(120 / portTICK_PERIOD_MS);
    }

    Splash::step(Data.getCurrent().valid ? "Ready" : "Starting offline", 100);
    // Apply the freshly-arrived data while the splash still covers the
    // screen, so the first revealed frame is already populated (and the
    // sky background matches the current conditions).
    UI::loop();
    Splash::hide();

    Serial.println("[GUITask] Ready");

    uint32_t fpsWindowStart = millis();
    uint32_t frameCount = 0;
    
    for (;;) {
        lv_timer_handler();
        UI::loop();
        Backlight::loop();
        frameCount++;
        uint32_t now = millis();
        if (now - fpsWindowStart >= 1000) {
            float fps = (frameCount * 1000.0f) / (float)(now - fpsWindowStart);
            Data.setDiagFps(fps);
            frameCount = 0;
            fpsWindowStart = now;
        }
        vTaskDelay(5 / portTICK_PERIOD_MS);
    }
}
