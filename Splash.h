#pragma once

// ============================================================
// Boot splash — branded loading screen for SmartClockOS
//
// Fixes the ugly power-on sequence. Previously the backlight came up at
// full brightness BEFORE the RGB panel had anything valid to scan, so the
// first thing you saw was the panel DMA-ing uninitialized PSRAM: coloured
// static, then a black flash, then the UI popping in piece by piece.
//
// Now: backlight starts at 0 (panel garbage happens in the dark), the
// splash is drawn and rendered first, and only THEN does the backlight
// fade up — so the first thing visible is a finished, branded frame.
//
// Lives on lv_layer_top() (same trick as the settings modal) so UI::build()
// still owns the active screen and needs no changes.
// ============================================================

#include "Config.h"
#include "Theme.h"
#include "WeatherVisuals.h"
#include <Arduino.h>
#include <esp_heap_caps.h>
#include <lvgl.h>

namespace Splash {

static const int LOGO_PX = 132; // square, ARGB8888

static lv_obj_t *root = nullptr;
static lv_obj_t *statusLabel = nullptr;
static lv_obj_t *barFill = nullptr;
static lv_obj_t *logoImg = nullptr;
static uint8_t *logoBuf = nullptr;
static lv_image_dsc_t logoDsc;
static uint32_t shownAt = 0;

static const int BAR_W = 240;

// ---- procedural logo -------------------------------------------------
// A clock bezel with hands, drawn with the same anti-aliased toolkit the
// weather art uses so the branding doesn't look bolted on. Alpha channel
// means it composites cleanly over the background.
static void drawLogo(uint32_t *px) {
  using namespace WeatherVisuals;
  CanvasA c = {px, LOGO_PX, LOGO_PX};
  memset(px, 0, (size_t)LOGO_PX * LOGO_PX * sizeof(uint32_t));

  const float cx = LOGO_PX / 2.0f;
  const float cy = LOGO_PX / 2.0f;

  const uint32_t accent = 0x3AA7FF;
  const uint32_t warm = 0xFFB84D;
  const uint32_t face = 0x151C24;
  const uint32_t faceEdge = 0x0E141B;
  const uint32_t textCol = 0xE6EEF7;

  // soft outer halo
  aaGlow(c, cx, cy, 63.0f, rgba(accent, 70));

  // bezel ring: accent disc, then punch the face out of the middle
  aaCircle(c, cx, cy, 57.0f, rgba(accent, 255));
  aaShadedDisc(c, cx, cy, 50.0f, rgba(face, 255), rgba(faceEdge, 255));

  // hour ticks at 12/3/6/9
  for (int i = 0; i < 4; i++) {
    float a = (float)i * 90.0f * 0.017453292f;
    float sx = cx + sinf(a) * 41.0f;
    float sy = cy - cosf(a) * 41.0f;
    float ex = cx + sinf(a) * 33.0f;
    float ey = cy - cosf(a) * 33.0f;
    aaCapsule(c, sx, sy, ex, ey, 2.4f, rgba(textCol, 210));
  }

  // hands parked at a deliberate 10:09 — the classic watch-advert pose
  float ha = -55.0f * 0.017453292f; // hour, ~10 o'clock
  aaCapsule(c, cx, cy, cx + sinf(ha) * 26.0f, cy - cosf(ha) * 26.0f, 3.6f,
            rgba(textCol, 255));
  float ma = 55.0f * 0.017453292f; // minute, ~2 o'clock
  aaCapsule(c, cx, cy, cx + sinf(ma) * 36.0f, cy - cosf(ma) * 36.0f, 3.0f,
            rgba(textCol, 255));

  // warm centre pin, tying it to the weather palette
  aaCircle(c, cx, cy, 5.6f, rgba(warm, 255));
  aaCircle(c, cx, cy, 2.4f, rgba(faceEdge, 255));
}

static bool buildLogo() {
  const uint32_t bytes = (uint32_t)LOGO_PX * LOGO_PX * sizeof(uint32_t);
  logoBuf = (uint8_t *)heap_caps_malloc(bytes, MALLOC_CAP_SPIRAM | MALLOC_CAP_8BIT);
  if (!logoBuf)
    return false;
  drawLogo((uint32_t *)logoBuf);
  WeatherVisuals::initImageDsc(logoDsc, LV_COLOR_FORMAT_ARGB8888, LOGO_PX,
                               LOGO_PX, LOGO_PX * 4, logoBuf, bytes);
  return true;
}

// ---- public ----------------------------------------------------------

void show() {
#if !ENABLE_SPLASH
  return; // flag off: no overlay, and every other call no-ops on root==null
#endif
  root = lv_obj_create(lv_layer_top());
  lv_obj_set_size(root, SCREEN_WIDTH, SCREEN_HEIGHT);
  lv_obj_set_pos(root, 0, 0);
  lv_obj_set_style_bg_color(root, Theme::COLOR_BG, 0);
  lv_obj_set_style_bg_opa(root, LV_OPA_COVER, 0);
  lv_obj_set_style_border_width(root, 0, 0);
  lv_obj_set_style_radius(root, 0, 0);
  lv_obj_set_style_pad_all(root, 0, 0);
  lv_obj_remove_flag(root, LV_OBJ_FLAG_SCROLLABLE);
  lv_obj_add_flag(root, LV_OBJ_FLAG_CLICKABLE); // swallow stray touches

  if (buildLogo()) {
    logoImg = lv_image_create(root);
    lv_image_set_src(logoImg, &logoDsc);
    lv_obj_align(logoImg, LV_ALIGN_CENTER, 0, -96);
  }

  // Wordmark as a shrink-to-fit flex row so it centres itself exactly —
  // no hand-tuned offsets that drift when the font metrics change.
  lv_obj_t *mark = lv_obj_create(root);
  lv_obj_remove_style_all(mark);
  lv_obj_set_size(mark, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
  lv_obj_set_flex_flow(mark, LV_FLEX_FLOW_ROW);
  lv_obj_set_flex_align(mark, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_END,
                        LV_FLEX_ALIGN_CENTER);
  lv_obj_set_style_pad_column(mark, 2, 0);
  lv_obj_remove_flag(mark, LV_OBJ_FLAG_SCROLLABLE);
  lv_obj_align(mark, LV_ALIGN_CENTER, 0, 14);

  lv_obj_t *title = lv_label_create(mark);
  lv_label_set_text(title, "SmartClock");
  lv_obj_set_style_text_font(title, &lv_font_montserrat_48, 0);
  lv_obj_set_style_text_color(title, Theme::COLOR_TEXT, 0);

  // "OS" in accent so the wordmark reads as one lockup
  lv_obj_t *os = lv_label_create(mark);
  lv_label_set_text(os, "OS");
  lv_obj_set_style_text_font(os, &lv_font_montserrat_48, 0);
  lv_obj_set_style_text_color(os, Theme::COLOR_ACCENT, 0);

  lv_obj_t *ver = lv_label_create(root);
  lv_label_set_text(ver, "v" CLOCK_VERSION);
  lv_obj_set_style_text_font(ver, &lv_font_montserrat_16, 0);
  lv_obj_set_style_text_color(ver, Theme::COLOR_SUBTEXT, 0);
  lv_obj_align(ver, LV_ALIGN_CENTER, 0, 56);

  // progress track + fill (plain objects: lighter than lv_bar and styled
  // to match the rest of the theme)
  lv_obj_t *track = lv_obj_create(root);
  lv_obj_set_size(track, BAR_W, 4);
  lv_obj_align(track, LV_ALIGN_CENTER, 0, 96);
  lv_obj_set_style_bg_color(track, Theme::COLOR_BORDER, 0);
  lv_obj_set_style_bg_opa(track, LV_OPA_COVER, 0);
  lv_obj_set_style_border_width(track, 0, 0);
  lv_obj_set_style_radius(track, 2, 0);
  lv_obj_set_style_pad_all(track, 0, 0);
  lv_obj_remove_flag(track, LV_OBJ_FLAG_SCROLLABLE);

  barFill = lv_obj_create(track);
  lv_obj_set_size(barFill, 0, 4);
  lv_obj_align(barFill, LV_ALIGN_LEFT_MID, 0, 0);
  lv_obj_set_style_bg_color(barFill, Theme::COLOR_ACCENT, 0);
  lv_obj_set_style_bg_opa(barFill, LV_OPA_COVER, 0);
  lv_obj_set_style_border_width(barFill, 0, 0);
  lv_obj_set_style_radius(barFill, 2, 0);
  lv_obj_set_style_pad_all(barFill, 0, 0);
  lv_obj_remove_flag(barFill, LV_OBJ_FLAG_SCROLLABLE);

  statusLabel = lv_label_create(root);
  lv_label_set_text(statusLabel, "Starting up");
  lv_obj_set_style_text_font(statusLabel, &lv_font_montserrat_16, 0);
  lv_obj_set_style_text_color(statusLabel, Theme::COLOR_SUBTEXT, 0);
  lv_obj_align(statusLabel, LV_ALIGN_CENTER, 0, 120);

  shownAt = millis();
}

// Update the caption and progress, then push a frame so the change is
// actually visible before the next blocking init step begins.
void step(const char *msg, int pct) {
  if (!root)
    return;
  if (statusLabel && msg)
    lv_label_set_text(statusLabel, msg);
  if (barFill) {
    if (pct < 0) pct = 0;
    if (pct > 100) pct = 100;
    lv_obj_set_width(barFill, (BAR_W * pct) / 100);
  }
  static const char *lastLogged = nullptr;
  if (msg && msg != lastLogged) {
    WebConfig::logPrintf("[Splash] %s (%d%%)\n", msg, pct);
    lastLogged = msg;
  }
  lv_timer_handler();
}

bool active() { return root != nullptr; }

// Fade the overlay out to reveal the finished UI, then reclaim the logo.
void hide(uint32_t fadeMs = 320) {
  if (!root)
    return;
  WebConfig::logPrintf("[Splash] hide\n");

  // Guarantee a minimum on-screen time so a fast boot doesn't just flash.
  const uint32_t MIN_VISIBLE_MS = 1100;
  uint32_t elapsed = millis() - shownAt;
  while (elapsed < MIN_VISIBLE_MS) {
    lv_timer_handler();
    vTaskDelay(20 / portTICK_PERIOD_MS);
    elapsed = millis() - shownAt;
  }

  const int steps = 16;
  for (int i = steps; i >= 0; i--) {
    lv_obj_set_style_opa(root, (lv_opa_t)((255 * i) / steps), 0);
    lv_timer_handler();
    vTaskDelay((fadeMs / steps) / portTICK_PERIOD_MS);
  }

  lv_obj_delete(root);
  root = nullptr;
  statusLabel = nullptr;
  barFill = nullptr;
  logoImg = nullptr;

  lv_timer_handler();

  // The logo is only ever needed once — hand the PSRAM back.
  if (logoBuf) {
    heap_caps_free(logoBuf);
    logoBuf = nullptr;
  }
  WebConfig::logPrintf("[Splash] hidden\n");
}

} // namespace Splash
