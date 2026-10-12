#pragma once

#include "Config.h"
#include "WebConfig.h"
#include <Arduino.h>
#include <esp_heap_caps.h>
#include <lvgl.h>
#include <ctype.h>
#include <math.h>
#include <string.h>

// Procedural weather art, rendered once at boot into PSRAM.
// Everything is drawn with anti-aliased distance-field shapes — no bitmap
// assets. Icons and sprites are ARGB8888; hero backgrounds are RGB565
// (rendered in ARGB first, then quantized with ordered dithering to avoid
// gradient banding).

namespace WeatherVisuals {

enum VisualMode : uint8_t {
    VIS_SUN = 0,
    VIS_CLOUD = 1,
    VIS_RAIN = 2,
    VIS_NIGHT = 3,
    VIS_SNOW = 4,
    VIS_THUNDER = 5,
    VIS_FOG = 6,
    VIS_PARTLY_CLOUDY = 7,
    VIS_COUNT = 8
};

// Animated sprites used by the hero section (UI.h moves these around).
enum SpriteId : uint8_t {
    SPR_SUN = 0,         // 120x120 sun disc + rays + glow
    SPR_MOON = 1,        // 100x100 crescent + craters + glow
    SPR_CLOUD = 2,       // 150x54 puffy cloud (near layer)
    SPR_RAIN = 3,        // 6x22 raindrop streak
    SPR_SNOW = 4,        // 14x14 snowflake
    SPR_STAR = 5,        // 12x12 four-point sparkle
    SPR_FOG = 6,         // 320x40 soft mist band
    SPR_BOLT = 7,        // 44x92 lightning bolt
    SPR_CLOUD_FAR = 8,   // 100x36 distant cloud (far layer)
    SPR_SHOOT = 9,       // 48x16 shooting star streak
    SPR_BIRD = 10,       // 20x10 bird silhouette
    SPR_STAR_S = 11,     // 9x9 small sparkle
    SPR_STAR_L = 12,     // 18x18 large sparkle
    SPR_SNOW_S = 13,     // 10x10 small flake
    SPR_SNOW_L = 14,     // 20x20 large flake
    SPR_CLOUD_BIG = 15,  // 200x72 large cloud
    SPR_COUNT = 16
};

static constexpr int ICON_W = 64;
static constexpr int ICON_H = 64;
static constexpr int HERO_BG_W = SCREEN_WIDTH;
static constexpr int HERO_BG_H = SCREEN_HEIGHT;

static const uint16_t SPRITE_W[SPR_COUNT] = {120, 100, 150, 6, 14, 12, 320, 44, 100, 48, 20, 9, 18, 10, 20, 200};
static const uint16_t SPRITE_H[SPR_COUNT] = {120, 100, 54, 22, 14, 12, 40, 92, 36, 16, 10, 9, 18, 10, 20, 72};

struct CacheState {
    bool ready = false;
    uint8_t* iconBuf[VIS_COUNT] = {nullptr};
    uint8_t* sprBuf[SPR_COUNT] = {nullptr};
    uint8_t* bgBuf = nullptr;
    uint32_t* bgScratch = nullptr;
    int8_t bgMode = -1;
    lv_image_dsc_t icons[VIS_COUNT];
    lv_image_dsc_t background;
    lv_image_dsc_t sprites[SPR_COUNT];
};

static CacheState cache;

// ---------------------------------------------------------------------------
// ARGB8888 canvas + anti-aliased drawing toolkit
// ---------------------------------------------------------------------------

struct CanvasA {
    uint32_t* px;
    int w, h;
};

static inline uint32_t argb(uint8_t a, uint8_t r, uint8_t g, uint8_t b) {
    return ((uint32_t)a << 24) | ((uint32_t)r << 16) | ((uint32_t)g << 8) | (uint32_t)b;
}

// 0xRRGGBB + separate alpha convenience
static inline uint32_t rgba(uint32_t rgb, uint8_t a) {
    return ((uint32_t)a << 24) | (rgb & 0xFFFFFF);
}

static inline float clamp01f(float v) { return v < 0.0f ? 0.0f : (v > 1.0f ? 1.0f : v); }

// src-over blend of `col` at fractional coverage `cov` (0..1)
static inline void blendA(CanvasA& c, int x, int y, uint32_t col, float cov) {
    if (x < 0 || y < 0 || x >= c.w || y >= c.h) return;
    uint32_t sa = (uint32_t)(((col >> 24) & 0xFF) * cov + 0.5f);
    if (sa == 0) return;
    if (sa > 255) sa = 255;
    uint32_t sr = (col >> 16) & 0xFF, sg = (col >> 8) & 0xFF, sb = col & 0xFF;
    uint32_t& d = c.px[y * c.w + x];
    uint32_t da = (d >> 24) & 0xFF, dr = (d >> 16) & 0xFF, dg = (d >> 8) & 0xFF, db = d & 0xFF;
    uint32_t inv = 255 - sa;
    uint32_t oa = sa + (da * inv) / 255;
    if (oa == 0) { d = 0; return; }
    uint32_t orr = (sr * sa + (dr * da * inv) / 255) / oa;
    uint32_t og = (sg * sa + (dg * da * inv) / 255) / oa;
    uint32_t ob = (sb * sa + (db * da * inv) / 255) / oa;
    d = (oa << 24) | (orr << 16) | (og << 8) | ob;
}

static void aaCircle(CanvasA& c, float cx, float cy, float r, uint32_t col) {
    int x0 = (int)floorf(cx - r - 1), x1 = (int)ceilf(cx + r + 1);
    int y0 = (int)floorf(cy - r - 1), y1 = (int)ceilf(cy + r + 1);
    for (int y = y0; y <= y1; y++) {
        for (int x = x0; x <= x1; x++) {
            float dx = x - cx, dy = y - cy;
            float cov = clamp01f(r + 0.5f - sqrtf(dx * dx + dy * dy));
            if (cov > 0.003f) blendA(c, x, y, col, cov);
        }
    }
}

// Disc with a radial two-color shade (bright core -> darker rim)
static void aaShadedDisc(CanvasA& c, float cx, float cy, float r, uint32_t inner, uint32_t outer) {
    int x0 = (int)floorf(cx - r - 1), x1 = (int)ceilf(cx + r + 1);
    int y0 = (int)floorf(cy - r - 1), y1 = (int)ceilf(cy + r + 1);
    uint32_t ia = (inner >> 24) & 0xFF;
    for (int y = y0; y <= y1; y++) {
        for (int x = x0; x <= x1; x++) {
            float dx = x - cx, dy = y - cy;
            float d = sqrtf(dx * dx + dy * dy);
            float cov = clamp01f(r + 0.5f - d);
            if (cov <= 0.003f) continue;
            float t = clamp01f(d / r);
            t = t * t; // keep the bright core wide
            uint32_t rr = (uint32_t)((1 - t) * ((inner >> 16) & 0xFF) + t * ((outer >> 16) & 0xFF));
            uint32_t gg = (uint32_t)((1 - t) * ((inner >> 8) & 0xFF) + t * ((outer >> 8) & 0xFF));
            uint32_t bb = (uint32_t)((1 - t) * (inner & 0xFF) + t * (outer & 0xFF));
            blendA(c, x, y, (ia << 24) | (rr << 16) | (gg << 8) | bb, cov);
        }
    }
}

// Soft radial glow: alpha fades quadratically from center to r
static void aaGlow(CanvasA& c, float cx, float cy, float r, uint32_t col) {
    int x0 = (int)floorf(cx - r), x1 = (int)ceilf(cx + r);
    int y0 = (int)floorf(cy - r), y1 = (int)ceilf(cy + r);
    for (int y = y0; y <= y1; y++) {
        for (int x = x0; x <= x1; x++) {
            float dx = x - cx, dy = y - cy;
            float d = sqrtf(dx * dx + dy * dy);
            if (d >= r) continue;
            float t = 1.0f - d / r;
            blendA(c, x, y, col, t * t);
        }
    }
}

// Thick anti-aliased line with round caps
static void aaCapsule(CanvasA& c, float x0f, float y0f, float x1f, float y1f, float r, uint32_t col) {
    float minx = fminf(x0f, x1f) - r - 1, maxx = fmaxf(x0f, x1f) + r + 1;
    float miny = fminf(y0f, y1f) - r - 1, maxy = fmaxf(y0f, y1f) + r + 1;
    float vx = x1f - x0f, vy = y1f - y0f;
    float len2 = vx * vx + vy * vy;
    for (int y = (int)floorf(miny); y <= (int)ceilf(maxy); y++) {
        for (int x = (int)floorf(minx); x <= (int)ceilf(maxx); x++) {
            float px = x - x0f, py = y - y0f;
            float t = len2 > 0 ? clamp01f((px * vx + py * vy) / len2) : 0;
            float dx = px - t * vx, dy = py - t * vy;
            float cov = clamp01f(r + 0.5f - sqrtf(dx * dx + dy * dy));
            if (cov > 0.003f) blendA(c, x, y, col, cov);
        }
    }
}

// Crescent: inside main circle, outside cutout circle
static void aaCrescent(CanvasA& c, float cx, float cy, float r,
                       float cutDx, float cutDy, float cutR, uint32_t col) {
    int x0 = (int)floorf(cx - r - 1), x1 = (int)ceilf(cx + r + 1);
    int y0 = (int)floorf(cy - r - 1), y1 = (int)ceilf(cy + r + 1);
    float ccx = cx + cutDx, ccy = cy + cutDy;
    for (int y = y0; y <= y1; y++) {
        for (int x = x0; x <= x1; x++) {
            float dx = x - cx, dy = y - cy;
            float d1 = sqrtf(dx * dx + dy * dy);
            float in1 = clamp01f(r + 0.5f - d1);
            if (in1 <= 0.003f) continue;
            float ex = x - ccx, ey = y - ccy;
            float d2 = sqrtf(ex * ex + ey * ey);
            float out2 = clamp01f(d2 - (cutR - 0.5f));
            float cov = in1 * out2;
            if (cov > 0.003f) blendA(c, x, y, col, cov);
        }
    }
}

// Four-point sparkle (long vertical + horizontal spikes)
static void aaSparkle(CanvasA& c, float cx, float cy, float len, float thick, uint32_t col) {
    int x0 = (int)(cx - len - 1), x1 = (int)(cx + len + 1);
    int y0 = (int)(cy - len - 1), y1 = (int)(cy + len + 1);
    for (int y = y0; y <= y1; y++) {
        for (int x = x0; x <= x1; x++) {
            float ax = fabsf(x - cx), ay = fabsf(y - cy);
            float covV = clamp01f(1.0f - ax / thick) * clamp01f(1.0f - ay / len);
            float covH = clamp01f(1.0f - ay / thick) * clamp01f(1.0f - ax / len);
            float cov = fmaxf(covV * covV, covH * covH);
            if (cov > 0.003f) blendA(c, x, y, col, cov);
        }
    }
}

// Puffy two-tone cloud. (x,y) = top-left of bounding area, s = width in px.
// Drawn as a shaded base layer with lighter lobes on top.
static void aaCloud(CanvasA& c, float x, float y, float s, uint32_t light, uint32_t shade) {
    struct Lobe { float lx, ly, lr; };
    static const Lobe lobes[4] = {
        {0.20f, 0.26f, 0.15f},
        {0.40f, 0.16f, 0.20f},
        {0.62f, 0.22f, 0.17f},
        {0.79f, 0.30f, 0.12f},
    };
    // shaded silhouette, nudged down for a bottom rim
    for (const Lobe& L : lobes)
        aaCircle(c, x + L.lx * s, y + (L.ly + 0.045f) * s, L.lr * s, shade);
    aaCapsule(c, x + 0.17f * s, y + 0.37f * s, x + 0.82f * s, y + 0.37f * s, 0.115f * s, shade);
    // lit body
    for (const Lobe& L : lobes)
        aaCircle(c, x + L.lx * s, y + L.ly * s, L.lr * s * 0.93f, light);
    aaCapsule(c, x + 0.19f * s, y + 0.345f * s, x + 0.80f * s, y + 0.345f * s, 0.10f * s, light);
}

// Six-arm snowflake
static void aaFlake(CanvasA& c, float cx, float cy, float arm, float thick, uint32_t col) {
    for (int k = 0; k < 3; k++) {
        float a = (float)k * 1.0471976f; // 60 degrees
        float dx = cosf(a) * arm, dy = sinf(a) * arm;
        aaCapsule(c, cx - dx, cy - dy, cx + dx, cy + dy, thick, col);
    }
    aaCircle(c, cx, cy, thick * 1.4f, col);
}

// Cartoon lightning bolt with glow
static void aaBolt(CanvasA& c, float cx, float topY, float botY, float r, uint32_t core, uint32_t glow) {
    float midY = topY + (botY - topY) * 0.46f;
    aaGlow(c, cx - 2, midY, (botY - topY) * 0.55f, glow);
    aaCapsule(c, cx + 5, topY, cx - 3, midY, r, core);
    aaCapsule(c, cx - 3, midY, cx + 4, midY, r * 0.9f, core);
    aaCapsule(c, cx + 4, midY, cx - 5, botY, r * 0.75f, core);
}

// ---------------------------------------------------------------------------
// Vertical multi-stop gradient (opaque) for backgrounds
// ---------------------------------------------------------------------------

static void gradientV(CanvasA& c, const uint32_t* stops, const float* pos, int n) {
    for (int y = 0; y < c.h; y++) {
        float t = (float)y / (float)(c.h - 1);
        int seg = 0;
        while (seg < n - 2 && t > pos[seg + 1]) seg++;
        float span = pos[seg + 1] - pos[seg];
        float f = span > 0 ? clamp01f((t - pos[seg]) / span) : 0;
        uint32_t c0 = stops[seg], c1 = stops[seg + 1];
        uint32_t r = (uint32_t)((1 - f) * ((c0 >> 16) & 0xFF) + f * ((c1 >> 16) & 0xFF));
        uint32_t g = (uint32_t)((1 - f) * ((c0 >> 8) & 0xFF) + f * ((c1 >> 8) & 0xFF));
        uint32_t b = (uint32_t)((1 - f) * (c0 & 0xFF) + f * (c1 & 0xFF));
        uint32_t col = 0xFF000000u | (r << 16) | (g << 8) | b;
        for (int x = 0; x < c.w; x++) c.px[y * c.w + x] = col;
    }
}

// ARGB scratch -> RGB565 with 4x4 Bayer ordered dithering (kills banding)
static void quantizeDither565(const CanvasA& src, uint16_t* dst) {
    static const int8_t bayer[4][4] = {
        {-8, 0, -6, 2}, {4, -4, 6, -2}, {-5, 3, -7, 1}, {7, -1, 5, -3}};
    for (int y = 0; y < src.h; y++) {
        for (int x = 0; x < src.w; x++) {
            uint32_t p = src.px[y * src.w + x];
            int n = bayer[y & 3][x & 3];
            int r = (int)((p >> 16) & 0xFF) + n;
            int g = (int)((p >> 8) & 0xFF) + (n >> 1);
            int b = (int)(p & 0xFF) + n;
            r = r < 0 ? 0 : (r > 255 ? 255 : r);
            g = g < 0 ? 0 : (g > 255 ? 255 : g);
            b = b < 0 ? 0 : (b > 255 ? 255 : b);
            dst[y * src.w + x] = (uint16_t)(((r & 0xF8) << 8) | ((g & 0xFC) << 3) | (b >> 3));
        }
    }
}

// ---------------------------------------------------------------------------
// Icons (64x64, emoji-style)
// ---------------------------------------------------------------------------

static void generateIcon(VisualMode mode, uint32_t* buf) {
    CanvasA c = {buf, ICON_W, ICON_H};
    memset(buf, 0, ICON_W * ICON_H * sizeof(uint32_t));

    const uint32_t CLOUD_LT = rgba(0xF2F7FD, 255);
    const uint32_t CLOUD_SH = rgba(0xBFD1E4, 255);
    const uint32_t STORM_LT = rgba(0x5C6678, 255);
    const uint32_t STORM_SH = rgba(0x414A5A, 255);

    switch (mode) {
    case VIS_SUN: {
        aaGlow(c, 32, 32, 30, rgba(0xFFD75E, 80));
        for (int i = 0; i < 8; i++) {
            float a = (float)i * 0.7853982f;
            aaCapsule(c, 32 + 21.0f * cosf(a), 32 + 21.0f * sinf(a),
                      32 + 29.0f * cosf(a), 32 + 29.0f * sinf(a), 2.6f, rgba(0xFFC93C, 255));
        }
        aaShadedDisc(c, 32, 32, 16, rgba(0xFFF6C4, 255), rgba(0xFFB52E, 255));
        break;
    }
    case VIS_CLOUD: {
        aaCloud(c, 14, 8, 46, rgba(0xCBD9E9, 255), rgba(0xA9BCD3, 255));
        aaCloud(c, 2, 20, 58, CLOUD_LT, CLOUD_SH);
        break;
    }
    case VIS_RAIN: {
        aaCloud(c, 6, 4, 54, rgba(0xC4D3E5, 255), rgba(0x9DB2CA, 255));
        for (int i = 0; i < 3; i++) {
            float x = 18 + i * 13;
            aaCapsule(c, x + 2, 38, x - 2, 52, 2.4f, rgba(0x54B4FF, 235));
        }
        break;
    }
    case VIS_NIGHT: {
        aaGlow(c, 30, 32, 26, rgba(0xC8D8FF, 60));
        aaCrescent(c, 30, 32, 17, 10, -7, 15, rgba(0xF2EFD5, 255));
        aaCircle(c, 25, 30, 2.4f, rgba(0xD9D5B8, 120));
        aaCircle(c, 30, 40, 1.7f, rgba(0xD9D5B8, 110));
        aaSparkle(c, 51, 14, 6, 1.5f, rgba(0xCFE0FF, 235));
        aaSparkle(c, 55, 30, 4, 1.2f, rgba(0xB9CEF5, 200));
        break;
    }
    case VIS_SNOW: {
        aaCloud(c, 6, 2, 54, CLOUD_LT, CLOUD_SH);
        aaFlake(c, 24, 46, 8, 1.5f, rgba(0xFFFFFF, 245));
        aaFlake(c, 44, 50, 6, 1.3f, rgba(0xE4F0FF, 220));
        break;
    }
    case VIS_THUNDER: {
        aaCloud(c, 6, 2, 54, STORM_LT, STORM_SH);
        aaBolt(c, 32, 34, 58, 4.2f, rgba(0xFFDF3C, 255), rgba(0xFFDF3C, 70));
        break;
    }
    case VIS_FOG: {
        const float len[4] = {24, 20, 23, 16};
        const uint8_t alpha[4] = {230, 180, 210, 150};
        for (int i = 0; i < 4; i++) {
            float y = 18 + i * 10;
            float x0 = 32 - len[i], x1 = 32 + len[i];
            aaCapsule(c, x0 + (i % 2) * 5, y, x1 - ((i + 1) % 2) * 4, y, 3.4f,
                      rgba(0xC6D3E1, alpha[i]));
        }
        break;
    }
    case VIS_PARTLY_CLOUDY:
    default: {
        aaGlow(c, 42, 18, 18, rgba(0xFFD75E, 70));
        for (int i = 0; i < 8; i++) {
            float a = (float)i * 0.7853982f;
            aaCapsule(c, 42 + 13.5f * cosf(a), 18 + 13.5f * sinf(a),
                      42 + 19.0f * cosf(a), 18 + 19.0f * sinf(a), 1.9f, rgba(0xFFC93C, 255));
        }
        aaShadedDisc(c, 42, 18, 10, rgba(0xFFF6C4, 255), rgba(0xFFB52E, 255));
        aaCloud(c, 0, 24, 52, CLOUD_LT, CLOUD_SH);
        break;
    }
    }
}

// ---------------------------------------------------------------------------
// Hero backgrounds (sky scenes; sun/moon live in animated sprites now)
// ---------------------------------------------------------------------------

static void generateBackground(VisualMode mode, CanvasA& c, uint16_t* out565) {
    switch (mode) {
    case VIS_SUN: {
        const uint32_t stops[3] = {0x7CC4FF, 0x4FAEFF, 0x3A9BF5};
        const float pos[3] = {0.0f, 0.55f, 1.0f};
        gradientV(c, stops, pos, 3);
        aaCloud(c, 60, 388, 130, rgba(0xEAF4FF, 90), rgba(0xCADFF4, 60));
        aaCloud(c, 470, 410, 170, rgba(0xEAF4FF, 70), rgba(0xCADFF4, 50));
        break;
    }
    case VIS_PARTLY_CLOUDY: {
        const uint32_t stops[3] = {0x74C0FF, 0x54A7EF, 0x418FD8};
        const float pos[3] = {0.0f, 0.5f, 1.0f};
        gradientV(c, stops, pos, 3);
        aaCloud(c, 70, 372, 190, rgba(0xF2F8FE, 160), rgba(0xC8DAEC, 120));
        aaCloud(c, 420, 400, 150, rgba(0xEAF3FC, 120), rgba(0xC2D5E9, 90));
        break;
    }
    case VIS_CLOUD: {
        const uint32_t stops[3] = {0x8298B6, 0x66809F, 0x4E6685};
        const float pos[3] = {0.0f, 0.5f, 1.0f};
        gradientV(c, stops, pos, 3);
        aaCloud(c, 40, 40, 220, rgba(0xD9E4F1, 200), rgba(0xAEC1D8, 160));
        aaCloud(c, 330, 90, 260, rgba(0xCEDCEC, 180), rgba(0xA2B7D0, 150));
        aaCloud(c, 590, 50, 190, rgba(0xDCE7F3, 190), rgba(0xB3C6DB, 150));
        break;
    }
    case VIS_RAIN: {
        const uint32_t stops[3] = {0x46586F, 0x33445A, 0x222E40};
        const float pos[3] = {0.0f, 0.5f, 1.0f};
        gradientV(c, stops, pos, 3);
        aaCloud(c, 50, 26, 230, rgba(0x9FAFC2, 220), rgba(0x76889D, 190));
        aaCloud(c, 350, 20, 260, rgba(0x93A4B9, 210), rgba(0x6C7E94, 190));
        aaCloud(c, 600, 34, 180, rgba(0xA5B4C7, 200), rgba(0x7D8FA4, 180));
        for (int i = 0; i < 34; i++) {
            float x = 18 + (i * 97) % (HERO_BG_W - 30);
            float y = 60 + ((i * 53) % 360);
            uint8_t a = (uint8_t)(70 + (i * 37) % 70);
            aaCapsule(c, x + 2, y, x - 2, y + 14, 1.3f, rgba(0x86C7FF, a));
        }
        break;
    }
    case VIS_NIGHT: {
        const uint32_t stops[3] = {0x090D16, 0x131E33, 0x1E2F4E};
        const float pos[3] = {0.0f, 0.55f, 1.0f};
        gradientV(c, stops, pos, 3);
        for (int i = 0; i < 90; i++) {
            int x = (i * 89) % HERO_BG_W;
            int y = (i * 61) % (HERO_BG_H - 60);
            uint8_t a = (uint8_t)(60 + (i * 41) % 160);
            float r = 0.7f + (float)((i * 13) % 10) / 12.0f;
            aaCircle(c, (float)x, (float)y, r, rgba(0xD6E4FF, a));
        }
        aaSparkle(c, 150, 60, 5, 1.2f, rgba(0xE6EEFF, 190));
        aaSparkle(c, 420, 40, 6, 1.3f, rgba(0xE6EEFF, 170));
        aaSparkle(c, 300, 120, 4, 1.1f, rgba(0xC9DAFB, 150));
        break;
    }
    case VIS_SNOW: {
        const uint32_t stops[3] = {0x93A6BC, 0x74879E, 0x5A6C82};
        const float pos[3] = {0.0f, 0.5f, 1.0f};
        gradientV(c, stops, pos, 3);
        aaCloud(c, 60, 30, 220, rgba(0xEDF3FA, 220), rgba(0xC5D4E4, 180));
        aaCloud(c, 380, 24, 240, rgba(0xE5EDF6, 210), rgba(0xBCCCDE, 180));
        aaCloud(c, 620, 40, 160, rgba(0xF0F5FB, 210), rgba(0xCBD9E8, 170));
        for (int i = 0; i < 40; i++) {
            float x = 14 + (i * 83) % (HERO_BG_W - 24);
            float y = 70 + ((i * 47) % 360);
            uint8_t a = (uint8_t)(90 + (i * 29) % 120);
            aaCircle(c, x, y, 1.6f + (float)((i * 7) % 3) * 0.6f, rgba(0xFFFFFF, a));
        }
        break;
    }
    case VIS_THUNDER: {
        const uint32_t stops[3] = {0x2E2544, 0x201A31, 0x120E1D};
        const float pos[3] = {0.0f, 0.5f, 1.0f};
        gradientV(c, stops, pos, 3);
        aaCloud(c, 40, 20, 240, rgba(0x574F6B, 230), rgba(0x3D3752, 210));
        aaCloud(c, 340, 14, 270, rgba(0x4C4560, 230), rgba(0x353049, 210));
        aaCloud(c, 610, 30, 170, rgba(0x5B5370, 220), rgba(0x413B57, 200));
        aaBolt(c, 250, 110, 190, 5.0f, rgba(0xFFE24A, 210), rgba(0xFFE24A, 45));
        break;
    }
    case VIS_FOG:
    default: {
        const uint32_t stops[3] = {0x8792A0, 0x6C7885, 0x545F6C};
        const float pos[3] = {0.0f, 0.5f, 1.0f};
        gradientV(c, stops, pos, 3);
        for (int band = 0; band < 5; band++) {
            float yc = 90 + band * 80;
            uint8_t a = (uint8_t)(46 + (band % 3) * 18);
            float x0 = (band % 2) ? -40.0f : 60.0f;
            float x1 = (band % 2) ? HERO_BG_W - 80.0f : HERO_BG_W + 40.0f;
            aaCapsule(c, x0, yc, x1, yc, 15.0f + (band % 2) * 5.0f, rgba(0xDDE5EF, a));
        }
        break;
    }
    }
    quantizeDither565(c, out565);
}

// ---------------------------------------------------------------------------
// Animated sprites for the hero section
// ---------------------------------------------------------------------------

static void generateSprite(SpriteId id, uint32_t* buf) {
    CanvasA c = {buf, SPRITE_W[id], SPRITE_H[id]};
    memset(buf, 0, (size_t)SPRITE_W[id] * SPRITE_H[id] * sizeof(uint32_t));

    switch (id) {
    case SPR_SUN: {
        aaGlow(c, 60, 60, 58, rgba(0xFFD75E, 90));
        for (int i = 0; i < 8; i++) {
            float a = (float)i * 0.7853982f + 0.3926991f;
            aaCapsule(c, 60 + 38.0f * cosf(a), 60 + 38.0f * sinf(a),
                      60 + 52.0f * cosf(a), 60 + 52.0f * sinf(a), 4.2f, rgba(0xFFC93C, 235));
        }
        aaShadedDisc(c, 60, 60, 30, rgba(0xFFF7C8, 255), rgba(0xFFB52E, 255));
        break;
    }
    case SPR_MOON: {
        aaGlow(c, 50, 50, 48, rgba(0xC9D9FF, 70));
        aaCrescent(c, 50, 50, 30, 17, -12, 27, rgba(0xF3F0D8, 250));
        aaCircle(c, 41, 46, 4.0f, rgba(0xDAD6B9, 120));
        aaCircle(c, 49, 62, 2.8f, rgba(0xDAD6B9, 110));
        aaCircle(c, 38, 60, 2.0f, rgba(0xDAD6B9, 95));
        break;
    }
    case SPR_CLOUD: {
        aaCloud(c, 4, 4, 140, rgba(0xF4F9FF, 225), rgba(0xC9D8EA, 190));
        break;
    }
    case SPR_RAIN: {
        aaCapsule(c, 3.0f, 3.0f, 3.0f, 12.0f, 1.4f, rgba(0x7BC7FF, 120));
        aaCapsule(c, 3.0f, 10.0f, 3.0f, 17.0f, 1.7f, rgba(0x7BC7FF, 235));
        aaCircle(c, 3.0f, 17.5f, 2.1f, rgba(0x9AD4FF, 245));
        break;
    }
    case SPR_SNOW: {
        aaFlake(c, 7, 7, 5.6f, 1.1f, rgba(0xF7FBFF, 235));
        break;
    }
    case SPR_STAR: {
        aaSparkle(c, 6, 6, 5.5f, 1.4f, rgba(0xE4EEFF, 255));
        break;
    }
    case SPR_FOG: {
        // soft horizontal mist band: gaussian-ish vertical profile, faded ends
        for (int y = 0; y < c.h; y++) {
            float dy = (y - c.h * 0.5f) / (c.h * 0.42f);
            float vy = expf(-dy * dy * 2.2f);
            for (int x = 0; x < c.w; x++) {
                float ex = 1.0f;
                if (x < 48) ex = x / 48.0f;
                else if (x > c.w - 48) ex = (c.w - x) / 48.0f;
                float a = 130.0f * vy * ex;
                if (a > 2) c.px[y * c.w + x] = argb((uint8_t)a, 0xDD, 0xE5, 0xEF);
            }
        }
        break;
    }
    case SPR_BOLT:
    default: {
        aaBolt(c, 22, 8, 84, 6.0f, rgba(0xFFE24A, 255), rgba(0xFFE24A, 80));
        break;
    }
    case SPR_CLOUD_FAR: {
        aaCloud(c, 2, 2, 96, rgba(0xF4F9FF, 215), rgba(0xC9D8EA, 185));
        break;
    }
    case SPR_SHOOT: {
        // Tapered streak, head at lower right so the tail trails behind.
        aaCapsule(c, 2, 3, 30, 8, 0.9f, rgba(0xFFFFFF, 90));
        aaCapsule(c, 8, 4.5f, 36, 9.5f, 1.4f, rgba(0xFFFFFF, 150));
        aaCapsule(c, 18, 6.5f, 42, 11, 1.9f, rgba(0xFFFDF0, 225));
        aaCircle(c, 44, 11.5f, 2.4f, rgba(0xFFFFFF, 255));
        break;
    }
    case SPR_BIRD: {
        const uint32_t ink = rgba(0x1E2733, 215);
        aaCapsule(c, 1, 6.5f, 9, 2.5f, 1.2f, ink);
        aaCapsule(c, 9, 2.5f, 18, 6.5f, 1.2f, ink);
        break;
    }
    case SPR_STAR_S: {
        aaSparkle(c, 4.5f, 4.5f, 4.0f, 1.1f, rgba(0xE4EEFF, 255));
        break;
    }
    case SPR_STAR_L: {
        aaSparkle(c, 9, 9, 8.0f, 2.0f, rgba(0xE4EEFF, 255));
        break;
    }
    case SPR_SNOW_S: {
        aaFlake(c, 5, 5, 3.8f, 0.9f, rgba(0xF7FBFF, 235));
        break;
    }
    case SPR_SNOW_L: {
        aaFlake(c, 10, 10, 8.0f, 1.5f, rgba(0xF7FBFF, 235));
        break;
    }
    case SPR_CLOUD_BIG: {
        aaCloud(c, 4, 4, 190, rgba(0xF4F9FF, 225), rgba(0xC9D8EA, 190));
        break;
    }
    }
}

// ---------------------------------------------------------------------------
// Condition mapping (unchanged behavior)
// ---------------------------------------------------------------------------

static inline void initImageDsc(lv_image_dsc_t& dsc, lv_color_format_t cf, uint32_t w, uint32_t h, uint32_t stride,
                                const uint8_t* data, uint32_t dataSize) {
    memset(&dsc, 0, sizeof(dsc));
    dsc.header.magic = LV_IMAGE_HEADER_MAGIC;
    dsc.header.cf = cf;
    dsc.header.flags = 0;
    dsc.header.w = w;
    dsc.header.h = h;
    dsc.header.stride = stride;
    dsc.data_size = dataSize;
    dsc.data = data;
}

static bool ciContains(const char* text, const char* token) {
    if (!text || !token) return false;
    size_t n = strlen(text);
    size_t m = strlen(token);
    if (m == 0 || n < m) return false;
    for (size_t i = 0; i <= n - m; i++) {
        bool ok = true;
        for (size_t j = 0; j < m; j++) {
            if (tolower((unsigned char)text[i + j]) != tolower((unsigned char)token[j])) {
                ok = false;
                break;
            }
        }
        if (ok) return true;
    }
    return false;
}

static VisualMode fromCondition(const char* condition, bool isDay, float precip = 0.0f) {
    if (!isDay) {
        if (ciContains(condition, "thunder") || ciContains(condition, "storm")) return VIS_THUNDER;
        if (ciContains(condition, "snow") || ciContains(condition, "blizzard")) return VIS_SNOW;
        if (ciContains(condition, "rain")) return VIS_RAIN;
        if (ciContains(condition, "fog") || ciContains(condition, "mist")) return VIS_FOG;
        return VIS_NIGHT;
    }
    if (ciContains(condition, "thunder") || ciContains(condition, "storm")) return VIS_THUNDER;
    if (ciContains(condition, "snow") || ciContains(condition, "blizzard")) return VIS_SNOW;
    if (ciContains(condition, "fog") || ciContains(condition, "mist")) return VIS_FOG;
    if (precip > 0.2f || ciContains(condition, "rain")) return VIS_RAIN;
    if (ciContains(condition, "partly") || ciContains(condition, "scattered")) return VIS_PARTLY_CLOUDY;
    if (ciContains(condition, "cloud") || ciContains(condition, "overcast")) return VIS_CLOUD;
    return VIS_SUN;
}

// ---------------------------------------------------------------------------
// Init & accessors
// ---------------------------------------------------------------------------

static bool init() {
    if (cache.ready) return true;

    uint32_t t0 = millis();
    WebConfig::logPrintf("[Visuals] Generating art...\n");
    const uint32_t iconBytes = ICON_W * ICON_H * sizeof(uint32_t);
    const uint32_t bgBytes = HERO_BG_W * HERO_BG_H * sizeof(uint16_t);

    // Full-screen sky: one shared ARGB scratch + one RGB565 canvas. The
    // background is (re)rendered on demand when the visual mode changes.
    cache.bgScratch = (uint32_t*)heap_caps_malloc(
        (size_t)HERO_BG_W * HERO_BG_H * sizeof(uint32_t), MALLOC_CAP_SPIRAM | MALLOC_CAP_8BIT);
    cache.bgBuf = (uint8_t*)heap_caps_malloc(bgBytes, MALLOC_CAP_SPIRAM | MALLOC_CAP_8BIT);
    if (!cache.bgScratch || !cache.bgBuf) {
        Serial.println("[Visuals] background alloc failed");
        return false;
    }
    initImageDsc(cache.background, LV_COLOR_FORMAT_RGB565, HERO_BG_W, HERO_BG_H,
                 HERO_BG_W * 2, cache.bgBuf, bgBytes);

    for (int i = 0; i < VIS_COUNT; i++) {
        cache.iconBuf[i] = (uint8_t*)heap_caps_malloc(iconBytes, MALLOC_CAP_SPIRAM | MALLOC_CAP_8BIT);
        if (!cache.iconBuf[i]) {
            Serial.println("[Visuals] icon alloc failed");
            return false;
        }
        generateIcon((VisualMode)i, (uint32_t*)cache.iconBuf[i]);
        initImageDsc(cache.icons[i], LV_COLOR_FORMAT_ARGB8888, ICON_W, ICON_H, ICON_W * 4,
                     cache.iconBuf[i], iconBytes);
    }
    WebConfig::logPrintf("[Visuals] icons (%lums)\n", (unsigned long)(millis() - t0));

    for (int i = 0; i < SPR_COUNT; i++) {
        uint32_t bytes = (uint32_t)SPRITE_W[i] * SPRITE_H[i] * sizeof(uint32_t);
        cache.sprBuf[i] = (uint8_t*)heap_caps_malloc(bytes, MALLOC_CAP_SPIRAM | MALLOC_CAP_8BIT);
        if (!cache.sprBuf[i]) {
            Serial.println("[Visuals] sprite alloc failed");
            return false;
        }
        generateSprite((SpriteId)i, (uint32_t*)cache.sprBuf[i]);
        initImageDsc(cache.sprites[i], LV_COLOR_FORMAT_ARGB8888, SPRITE_W[i], SPRITE_H[i],
                     SPRITE_W[i] * 4, cache.sprBuf[i], bytes);
    }

    cache.ready = true;
    WebConfig::logPrintf("[Visuals] ready (%lums)\n",
                         (unsigned long)(millis() - t0));
    return true;
}

static const lv_image_dsc_t* icon(VisualMode mode) {
    if (!cache.ready) return nullptr;
    int idx = (mode >= VIS_COUNT) ? 0 : (int)mode;
    return &cache.icons[idx];
}

// Full-screen sky for the given mode, rendered on demand into a single
// canvas; regenerated only when the mode actually changes.
static const lv_image_dsc_t* background(VisualMode mode) {
    if (!cache.ready || !cache.bgBuf || !cache.bgScratch) return nullptr;
    if (mode >= VIS_COUNT) mode = VIS_SUN;
    if (cache.bgMode == (int8_t)mode) return &cache.background;

    uint32_t t0 = millis();
    CanvasA c = {cache.bgScratch, HERO_BG_W, HERO_BG_H};
    generateBackground(mode, c, (uint16_t*)cache.bgBuf);
    cache.bgMode = (int8_t)mode;
    WebConfig::logPrintf("[Visuals] bg %d (%lums)\n", (int)mode,
                         (unsigned long)(millis() - t0));
    return &cache.background;
}

static const lv_image_dsc_t* sprite(SpriteId id) {
    if (!cache.ready) return nullptr;
    int idx = (id >= SPR_COUNT) ? 0 : (int)id;
    return &cache.sprites[idx];
}

}  // namespace WeatherVisuals
