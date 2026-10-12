#pragma once

#include "Config.h"
#include <Arduino.h>

namespace Backlight {

static int currentValue = BACKLIGHT_MAX;
static bool sleeping = false;
static int wakeValue = BACKLIGHT_MAX;
static bool nightActive = false;
static int targetBrightness = BACKLIGHT_MAX;
static int displayBrightness = BACKLIGHT_MAX;
static uint32_t lastTransitionMs = 0;
static const int TRANSITION_STEP_MS = 100;
static const int TRANSITION_DURATION_MS = 30000;

// Held true from power-on until the first finished frame exists. While set,
// brightness changes are recorded but NOT sent to the panel, so the RGB
// peripheral can scan uninitialized PSRAM (coloured static) in the dark
// instead of showing it to you. Cleared by fadeUp().
static bool bootHold = true;

static inline void writePwm(int v) {
    if (bootHold) return;
#if defined(ESP_ARDUINO_VERSION) && ESP_ARDUINO_VERSION >= ESP_ARDUINO_VERSION_VAL(3, 0, 0)
    ledcWrite(RGB_BL, v);
#else
    ledcWrite(0, v);
#endif
}

void init() {
    pinMode(RGB_BL, OUTPUT);
#if defined(ESP_ARDUINO_VERSION) && ESP_ARDUINO_VERSION >= ESP_ARDUINO_VERSION_VAL(3, 0, 0)
    ledcAttach(RGB_BL, BACKLIGHT_PWM_FREQ, BACKLIGHT_PWM_BITS);
    ledcWrite(RGB_BL, 0);
#else
    ledcSetup(0, BACKLIGHT_PWM_FREQ, BACKLIGHT_PWM_BITS);
    ledcAttachPin(RGB_BL, 0);
    ledcWrite(0, 0);
#endif
}

// Reveal the display by ramping 0 -> whatever brightness was requested
// while we were dark. Uses targetBrightness so a boot during night-mode
// hours fades straight to the dim level instead of blasting full white.
void fadeUp(uint32_t durationMs) {
    bootHold = false;
    int target = targetBrightness;
    if (target < 1) target = 1;
    displayBrightness = target;

    const int steps = 24;
    for (int i = 0; i <= steps; i++) {
        // ease-out so it blooms in rather than ramping linearly
        float t = (float)i / (float)steps;
        int v = (int)(target * (1.0f - (1.0f - t) * (1.0f - t)));
#if defined(ESP_ARDUINO_VERSION) && ESP_ARDUINO_VERSION >= ESP_ARDUINO_VERSION_VAL(3, 0, 0)
        ledcWrite(RGB_BL, v);
#else
        ledcWrite(0, v);
#endif
        vTaskDelay((durationMs / steps) / portTICK_PERIOD_MS);
    }
    writePwm(target);
}

void set(int value) {
    if (value < 0) value = 0;
    if (value > BACKLIGHT_MAX) value = BACKLIGHT_MAX;
    currentValue = value;
    if (!nightActive) {
        targetBrightness = value;
        displayBrightness = value;
    }
    if (sleeping) {
        wakeValue = value;
        return;
    }
    writePwm(displayBrightness);
}

int get() { return sleeping ? wakeValue : currentValue; }

void setNightMode(bool enabled, uint8_t nightBrightness) {
    nightActive = enabled;
    if (enabled) {
        targetBrightness = nightBrightness;
        if (targetBrightness < 10) targetBrightness = 10;
    } else {
        targetBrightness = currentValue;
    }
    lastTransitionMs = millis();
}

bool isNightMode() { return nightActive; }

void updateTransition() {
    if (sleeping) return;

    int diff = targetBrightness - displayBrightness;
    if (diff == 0) return;

    uint32_t now = millis();
    if (now - lastTransitionMs < TRANSITION_STEP_MS) return;
    lastTransitionMs = now;

    int stepsRemaining = abs(diff);
    int stepsPerUpdate = max(1, stepsRemaining / (TRANSITION_DURATION_MS / TRANSITION_STEP_MS));

    if (diff > 0) {
        displayBrightness = min(displayBrightness + stepsPerUpdate, targetBrightness);
    } else {
        displayBrightness = max(displayBrightness - stepsPerUpdate, targetBrightness);
    }

    writePwm(displayBrightness);
}

void loop() {
    updateTransition();
}

void sleep() {
    wakeValue = currentValue;
    sleeping = true;
    writePwm(0);
}

void wake() {
    sleeping = false;
    writePwm(displayBrightness);
}

bool isSleeping() { return sleeping; }

int getDisplayBrightness() { return displayBrightness; }

}
