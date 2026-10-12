#pragma once

#include "Config.h"
#include "Backlight.h"
#include "WebConfig.h"
#include <Arduino.h>
#include <TAMC_GT911.h>
#include <Wire.h>
#include <lvgl.h>

namespace Touch {

static TAMC_GT911* ts = nullptr;
static bool ready = false;
static bool seenTouch = false;
static uint32_t lastTouchLogMs = 0;

static void selectAddr(uint8_t addr) {
    pinMode(TOUCH_INT, OUTPUT);
    pinMode(TOUCH_RST, OUTPUT);
    digitalWrite(TOUCH_INT, LOW);
    digitalWrite(TOUCH_RST, LOW);
    delay(10);
    digitalWrite(TOUCH_INT, addr == TOUCH_ADDR_ALT ? HIGH : LOW);
    delay(1);
    digitalWrite(TOUCH_RST, HIGH);
    delay(5);
    digitalWrite(TOUCH_INT, LOW);
    delay(50);
    pinMode(TOUCH_INT, INPUT);
    delay(50);
}

static bool probeAddr(uint8_t addr) {
    Wire.beginTransmission(addr);
    return Wire.endTransmission() == 0;
}

static bool readId(uint8_t addr, uint8_t* out, size_t len) {
    if (len < 4) return false;
    Wire.beginTransmission(addr);
    Wire.write(highByte(GT911_PRODUCT_ID));
    Wire.write(lowByte(GT911_PRODUCT_ID));
    if (Wire.endTransmission() != 0) return false;
    Wire.requestFrom((int)addr, 4);
    if (Wire.available() < 4) return false;
    for (int i = 0; i < 4; i++) out[i] = (uint8_t)Wire.read();
    return true;
}

static uint8_t detectAddr() {
    uint8_t addrs[] = {TOUCH_ADDR, TOUCH_ADDR_ALT};
    uint8_t id[4];
    for (uint8_t a : addrs) {
        selectAddr(a);
        if (!probeAddr(a)) continue;
        if (readId(a, id, sizeof(id))) return a;
        return a;
    }
    return 0;
}

void init() {
    Wire.begin(TOUCH_SDA, TOUCH_SCL);
    Wire.setClock(100000);
    selectAddr(TOUCH_ADDR);
    
    uint8_t addr = detectAddr();

    if (!addr) {
        Serial.println("[Touch] GT911 not detected on 0x5D/0x14");
        ready = false;
        return;
    }

    uint8_t id[4] = {0};
    bool found = readId(addr, id, sizeof(id));
    
    if (found) {
        char idText[5] = {0};
        for (int i = 0; i < 4; i++) {
            idText[i] = (id[i] >= 32 && id[i] <= 126) ? (char)id[i] : '.';
        }
        Serial.printf("[Touch] GT911 at 0x%02X (ID: %02X %02X %02X %02X, '%s')\n",
                      addr, id[0], id[1], id[2], id[3], idText);
    } else {
        Serial.printf("[Touch] GT911 at 0x%02X (ID read failed)\n", addr);
    }
    
    ts = new TAMC_GT911(TOUCH_SDA, TOUCH_SCL, TOUCH_INT, TOUCH_RST, SCREEN_WIDTH, SCREEN_HEIGHT);
    ts->begin(addr);
    ts->setRotation(ROTATION_INVERTED);
    ready = true;
    
    Serial.println("[Touch] Initialized");
}

static void readCb(lv_indev_t* indev, lv_indev_data_t* data) {
    if (!ready || !ts) {
        data->state = LV_INDEV_STATE_RELEASED;
        return;
    }
    
    ts->read();
    
    if (ts->isTouched) {
        if (!seenTouch) {
            WebConfig::logPrintf("[Touch] First touch x=%d y=%d\n", ts->points[0].x, ts->points[0].y);
            seenTouch = true;
        }
        if (millis() - lastTouchLogMs > 1000) {
            lastTouchLogMs = millis();
            WebConfig::logPrintf("[Touch] Touch x=%d y=%d\n", ts->points[0].x, ts->points[0].y);
        }
        
        if (Backlight::isSleeping()) {
            Backlight::wake();
            data->state = LV_INDEV_STATE_RELEASED;
            return;
        }
        
        data->state = LV_INDEV_STATE_PRESSED;
        int x = ts->points[0].x;
        int y = ts->points[0].y;
        
        x = constrain(x, 0, SCREEN_WIDTH - 1);
        y = constrain(y, 0, SCREEN_HEIGHT - 1);
        
        data->point.x = (lv_coord_t)x;
        data->point.y = (lv_coord_t)y;
    } else {
        data->state = LV_INDEV_STATE_RELEASED;
    }
}

lv_indev_t* createInput(lv_display_t* disp) {
    if (!ready) return nullptr;
    lv_indev_t* indev = lv_indev_create();
    lv_indev_set_type(indev, LV_INDEV_TYPE_POINTER);
    lv_indev_set_read_cb(indev, readCb);
    lv_indev_set_display(indev, disp);
    return indev;
}

}
