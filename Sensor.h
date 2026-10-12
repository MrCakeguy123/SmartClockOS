#pragma once

#include "Config.h"
#include "DataManager.h"
#include "WebConfig.h"
#include <Adafruit_BME280.h>
#include <Arduino.h>
#include <Wire.h>
#include <math.h>

#if ENABLE_BME280

namespace Sensor {

static Adafruit_BME280 bme;
static bool ready = false;

// Read register 0xD0 (chip ID): 0x60 = BME280, 0x58 = BMP280 (no humidity).
static uint8_t readChipId(uint8_t addr) {
    Wire1.beginTransmission(addr);
    Wire1.write(0xD0);
    if (Wire1.endTransmission() != 0) return 0;
    if (Wire1.requestFrom((int)addr, 1) != 1) return 0;
    return (uint8_t)Wire1.read();
}

static const char *chipName(uint8_t id) {
    if (id == 0x60) return "BME280";
    if (id == 0x58) return "BMP280 (no humidity - needs Adafruit_BMP280 lib!)";
    if (id == 0x61) return "BME680?";
    return "unknown";
}

void init() {
    Wire1.begin(BME280_SDA, BME280_SCL);

    // Bus scan so wiring problems are visible in the web log (/api/logs).
    int found = 0;
    for (uint8_t a = 1; a < 127; a++) {
        Wire1.beginTransmission(a);
        if (Wire1.endTransmission() == 0) {
            uint8_t id = readChipId(a);
            if (id) {
                WebConfig::logPrintf("[Sensor] I2C 0x%02X responds, chip ID 0x%02X = %s\n",
                                     a, id, chipName(id));
            } else {
                WebConfig::logPrintf("[Sensor] I2C 0x%02X responds\n", a);
            }
            found++;
        }
    }
    if (!found) {
        WebConfig::logPrintf(
            "[Sensor] I2C scan: NO devices on SDA=%d SCL=%d - check wiring/power\n",
            BME280_SDA, BME280_SCL);
    }

    if (bme.begin(BME280_ADDR, &Wire1)) {
        ready = true;
        bme.setSampling(
            Adafruit_BME280::MODE_FORCED,
            Adafruit_BME280::SAMPLING_X1,
            Adafruit_BME280::SAMPLING_X1,
            Adafruit_BME280::SAMPLING_X1,
            Adafruit_BME280::FILTER_OFF
        );
        WebConfig::logPrintf("[Sensor] BME280 ready at 0x%02X\n", BME280_ADDR);
    } else if (bme.begin(0x77, &Wire1)) {
        ready = true;
        bme.setSampling(
            Adafruit_BME280::MODE_FORCED,
            Adafruit_BME280::SAMPLING_X1,
            Adafruit_BME280::SAMPLING_X1,
            Adafruit_BME280::SAMPLING_X1,
            Adafruit_BME280::FILTER_OFF
        );
        WebConfig::logPrintf("[Sensor] BME280 ready at 0x77\n");
    } else {
        WebConfig::logPrintf("[Sensor] BME280 not found on 0x76/0x77!\n");
        Data.setDiagSensorHealth(false, 0);
    }
}

void read() {
    if (!ready) {
        Data.setDiagSensorHealth(false, 0);
        return;
    }
    
    bme.takeForcedMeasurement();
    
    float tempC = bme.readTemperature();
    float humidity = bme.readHumidity();
    float pressure = bme.readPressure() / 100.0F;
    if (isnan(tempC) || isnan(humidity) || isnan(pressure)) {
        Data.setDiagSensorHealth(false, millis());
        WebConfig::logPrintf("[Sensor] Read failed (NaN)\n");
        return;
    }
    
    Settings s = Data.getSettings();
    float displayTemp = tempC;
    const char* unit = "C";
    
    if (s.tempUnit == TEMP_F) {
        displayTemp = tempC * 9.0f / 5.0f + 32.0f;
        unit = "F";
    }
    
    IndoorData d;
    d.temp = displayTemp;
    d.humidity = humidity;
    d.pressure = pressure;
    d.valid = true;
    
    snprintf(d.tempStr, sizeof(d.tempStr), "%.1f%s", displayTemp, unit);
    snprintf(d.humidityStr, sizeof(d.humidityStr), "%.0f%%", humidity);
    
    Data.setIndoor(d);
    Data.setDiagSensorHealth(true, millis());

    // Logging every 10s flooded the ring buffer and pushed real errors out
    // of it within ~20 minutes. Only log a meaningful change, or hourly.
    static float lastLoggedTemp = -1000.0f;
    static float lastLoggedHum = -1000.0f;
    static uint32_t lastLoggedMs = 0;
    uint32_t nowMs = millis();
    if (fabsf(displayTemp - lastLoggedTemp) >= 0.5f ||
        fabsf(humidity - lastLoggedHum) >= 3.0f ||
        nowMs - lastLoggedMs >= 3600000UL) {
        WebConfig::logPrintf("[Sensor] %.1f%s, %.0f%%, %.0f hPa\n", displayTemp, unit, humidity, pressure);
        lastLoggedTemp = displayTemp;
        lastLoggedHum = humidity;
        lastLoggedMs = nowMs;
    }
}

}

#endif
