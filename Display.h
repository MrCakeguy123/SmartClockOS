#pragma once

#include "Config.h"
#include "Backlight.h"
#include "WebConfig.h"
#include <Arduino.h>
#include <Arduino_GFX_Library.h>
#include <esp_heap_caps.h>
#include <lvgl.h>

// LVGL's built-in pool (LV_MEM_SIZE) is only 64 KB and this UI exhausts it:
// the software renderer's temporary transform buffers fail to allocate and
// LVGL asserts into an infinite loop. Route LVGL's allocator to the 8 MB
// PSRAM instead, same as Weather.h does for its JSON documents.
#if LV_USE_STDLIB_MALLOC == LV_STDLIB_CUSTOM
extern "C" {

void lv_mem_init(void) {}
void lv_mem_deinit(void) {}

lv_mem_pool_t lv_mem_add_pool(void *mem, size_t bytes) {
    LV_UNUSED(mem);
    LV_UNUSED(bytes);
    return nullptr;
}

void lv_mem_remove_pool(lv_mem_pool_t pool) { LV_UNUSED(pool); }

void *lv_malloc_core(size_t size) {
    void *p = heap_caps_malloc(size, MALLOC_CAP_SPIRAM | MALLOC_CAP_8BIT);
    if (!p)
        p = heap_caps_malloc(size, MALLOC_CAP_INTERNAL | MALLOC_CAP_8BIT);
    return p;
}

void *lv_realloc_core(void *p, size_t new_size) {
    void *q = heap_caps_realloc(p, new_size, MALLOC_CAP_SPIRAM | MALLOC_CAP_8BIT);
    if (!q)
        q = heap_caps_realloc(p, new_size, MALLOC_CAP_INTERNAL | MALLOC_CAP_8BIT);
    return q;
}

void lv_free_core(void *p) { heap_caps_free(p); }

void lv_mem_monitor_core(lv_mem_monitor_t *mon_p) {
    if (!mon_p)
        return;
    size_t total = heap_caps_get_total_size(MALLOC_CAP_SPIRAM);
    size_t freeSize = heap_caps_get_free_size(MALLOC_CAP_SPIRAM);
    mon_p->total_size = total;
    mon_p->free_size = freeSize;
    mon_p->free_biggest_size = heap_caps_get_largest_free_block(MALLOC_CAP_SPIRAM);
    mon_p->used_cnt = 0;
    mon_p->max_used = 0;
    mon_p->used_pct = total ? (uint8_t)(((total - freeSize) * 100) / total) : 0;
    mon_p->frag_pct = 0;
}

lv_result_t lv_mem_test_core(void) { return LV_RESULT_OK; }

} // extern "C"
#endif

namespace Display {

static Arduino_ESP32RGBPanel* rgbPanel = nullptr;
static Arduino_RGB_Display* gfx = nullptr;
static lv_display_t* lvDisplay = nullptr;
static bool directMode = false;
static uint32_t renderLines = 0;
static bool doubleBuffer = false;

static void flushCb(lv_display_t* disp, const lv_area_t* area, uint8_t* px) {
    static uint32_t flushCount = 0;
    flushCount++;
    uint32_t w = lv_area_get_width(area);
    uint32_t h = lv_area_get_height(area);
    bool trace = (flushCount <= 12);
    if (trace)
        WebConfig::logPrintf("[Disp] flush %lu in (%d,%d %dx%d)\n",
                             (unsigned long)flushCount, (int)area->x1,
                             (int)area->y1, (int)w, (int)h);
    gfx->draw16bitRGBBitmap(area->x1, area->y1, (uint16_t*)px, w, h);
    if (trace)
        WebConfig::logPrintf("[Disp] flush %lu done\n",
                             (unsigned long)flushCount);
    lv_display_flush_ready(disp);
}

void init() {
    rgbPanel = new Arduino_ESP32RGBPanel(
        RGB_DE, RGB_VSYNC, RGB_HSYNC, RGB_PCLK,
        RGB_R0, RGB_R1, RGB_R2, RGB_R3, RGB_R4,
        RGB_G0, RGB_G1, RGB_G2, RGB_G3, RGB_G4, RGB_G5,
        RGB_B0, RGB_B1, RGB_B2, RGB_B3, RGB_B4,
        1, RGB_H_FRONT, RGB_H_PULSE, RGB_H_BACK,
        1, RGB_V_FRONT, RGB_V_PULSE, RGB_V_BACK,
        RGB_PCLK_POL, RGB_PCLK_FREQ
    );
    
    gfx = new Arduino_RGB_Display(SCREEN_WIDTH, SCREEN_HEIGHT, rgbPanel);
    gfx->begin();
    gfx->fillScreen(0x0000);
    
    lv_init();
    lv_log_register_print_cb([](lv_log_level_t level, const char *buf) {
        WebConfig::logPrintf("[LVGL:%d] %s", (int)level, buf);
    });
    lv_tick_set_cb([]() { return millis(); });
    
    lvDisplay = lv_display_create(SCREEN_WIDTH, SCREEN_HEIGHT);
    if (!lvDisplay) {
        Serial.println("[Display] lv_display_create failed!");
        return;
    }
    lv_display_set_color_format(lvDisplay, LV_COLOR_FORMAT_RGB565);
    lv_display_set_flush_cb(lvDisplay, flushCb);

    // Prefer an INTERNAL-RAM draw buffer: LVGL software rendering into PSRAM
    // is several times slower per pixel than internal SRAM. Fall back to
    // PSRAM only if internal RAM is too tight. Single buffer — the flush is a
    // synchronous memcpy, so a second buffer buys nothing.
    struct BufOption { uint16_t lines; uint32_t caps; const char* name; };
    const BufOption candidates[] = {
        {80, MALLOC_CAP_SPIRAM | MALLOC_CAP_8BIT, "psram"},
        {160, MALLOC_CAP_SPIRAM | MALLOC_CAP_8BIT, "psram"},
        {40, MALLOC_CAP_SPIRAM | MALLOC_CAP_8BIT, "psram"},
        {40, MALLOC_CAP_INTERNAL | MALLOC_CAP_8BIT, "internal"},
        {32, MALLOC_CAP_INTERNAL | MALLOC_CAP_8BIT, "internal"},
        {24, MALLOC_CAP_INTERNAL | MALLOC_CAP_8BIT, "internal"},
    };
    lv_color_t* buf1 = nullptr;
    uint32_t chosenBytes = 0;
    const char* chosenKind = "none";

    for (const BufOption& opt : candidates) {
        uint32_t bytes = SCREEN_WIDTH * (uint32_t)opt.lines * sizeof(lv_color_t);
        // Keep at least 60 KB of internal heap free for WiFi/TLS.
        if ((opt.caps & MALLOC_CAP_INTERNAL) &&
            heap_caps_get_free_size(MALLOC_CAP_INTERNAL) < bytes + 60 * 1024) {
            continue;
        }
        buf1 = (lv_color_t*)heap_caps_malloc(bytes, opt.caps);
        if (!buf1) continue;

        chosenBytes = bytes;
        renderLines = opt.lines;
        chosenKind = opt.name;
        break;
    }

    if (!buf1) {
        Serial.println("[Display] Buffer allocation failed!");
        return;
    }

    doubleBuffer = false;
    lv_display_set_buffers(lvDisplay, buf1, nullptr, chosenBytes, LV_DISPLAY_RENDER_MODE_PARTIAL);
    directMode = false;
    WebConfig::logPrintf("[Display] Partial mode: %lu lines, %s buffer (%lu bytes)\n",
                  (unsigned long)renderLines, chosenKind,
                  (unsigned long)chosenBytes);
    
    lv_timer_t* refrTimer = lv_display_get_refr_timer(lvDisplay);
    if (refrTimer) lv_timer_set_period(refrTimer, UI_FRAME_MS);
    
    Serial.println("[Display] Initialized");
}

lv_display_t* getDisplay() { return lvDisplay; }
Arduino_RGB_Display* getGfx() { return gfx; }

// Raw RGB565 panel framebuffer (PSRAM). Used by the /api/screenshot route.
uint16_t* rawFramebuffer() { return gfx ? gfx->getFramebuffer() : nullptr; }
bool isDirect() { return directMode; }

}
