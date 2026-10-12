#pragma once

#include "Config.h"

#if ENABLE_FAN

#include "DataManager.h"
#include "WebConfig.h"
#include <Arduino.h>

// 30mm 5V case fan on a low-side MOSFET, PWM at 25kHz (inaudible).
// Thermostat runs off the ESP32-S3 die temperature with hysteresis;
// night mode keeps it silent unless the chip gets genuinely hot.
namespace Fan {

static bool running = false;

void init() {
    ledcAttach(FAN_PIN, FAN_PWM_FREQ, FAN_PWM_BITS);
    ledcWrite(FAN_PIN, 0);
    WebConfig::logPrintf("[Fan] PWM ready on IO%d\n", FAN_PIN);
}

void loop() {
    float t = temperatureRead(); // die temperature, degrees C

    if (running) {
        if (t <= FAN_OFF_C)
            running = false;
    } else {
        if (t >= FAN_ON_C)
            running = true;
    }

    uint8_t duty = 0;
    if (running) {
        float k = (t - FAN_OFF_C) / (FAN_MAX_C - FAN_OFF_C);
        if (k < 0) k = 0;
        if (k > 1) k = 1;
        duty = FAN_MIN_DUTY + (uint8_t)((100 - FAN_MIN_DUTY) * k);

        Settings s = Data.getSettings();
        if (isNightModeActiveNow(s) && t < FAN_NIGHT_OVERRIDE_C)
            duty = 0; // wall clock in a bedroom: silence beats a few degrees
    }

    ledcWrite(FAN_PIN, (duty * ((1 << FAN_PWM_BITS) - 1)) / 100);
    Data.setDiagFan(t, duty);
}

} // namespace Fan

#endif
