#pragma once

#include <lvgl.h>

namespace Theme {

static const lv_color_t COLOR_BG = LV_COLOR_MAKE(0x0B, 0x0F, 0x14);
static const lv_color_t COLOR_PANEL = LV_COLOR_MAKE(0x15, 0x1C, 0x24);
static const lv_color_t COLOR_PANEL_ALT = LV_COLOR_MAKE(0x10, 0x16, 0x1D);
static const lv_color_t COLOR_BORDER = LV_COLOR_MAKE(0x2A, 0x36, 0x45);
static const lv_color_t COLOR_TEXT = LV_COLOR_MAKE(0xE6, 0xEE, 0xF7);
static const lv_color_t COLOR_SUBTEXT = LV_COLOR_MAKE(0x9F, 0xB0, 0xC3);
static const lv_color_t COLOR_ACCENT = LV_COLOR_MAKE(0x3A, 0xA7, 0xFF);
static const lv_color_t COLOR_ACCENT_DARK = LV_COLOR_MAKE(0x1B, 0x3B, 0x5A);
static const lv_color_t COLOR_WARM = LV_COLOR_MAKE(0xFF, 0xB8, 0x4D);
static const lv_color_t COLOR_COOL = LV_COLOR_MAKE(0x6E, 0xC1, 0xFF);
static const lv_color_t COLOR_DAY_TOP = LV_COLOR_MAKE(0x5D, 0xB2, 0xFF);
static const lv_color_t COLOR_DAY_BOT = LV_COLOR_MAKE(0x3A, 0xA7, 0xFF);
static const lv_color_t COLOR_NIGHT_TOP = LV_COLOR_MAKE(0x0B, 0x0F, 0x14);
static const lv_color_t COLOR_NIGHT_BOT = LV_COLOR_MAKE(0x1A, 0x28, 0x47);
static const lv_color_t COLOR_SUNRISE = LV_COLOR_MAKE(0xFF, 0xD7, 0x00);
static const lv_color_t COLOR_SUNSET = LV_COLOR_MAKE(0xFF, 0x6B, 0x35);

static lv_style_t stylePanel;
static lv_style_t styleCard;
static lv_style_t styleBtn;

void init() {
    lv_style_init(&stylePanel);
    lv_style_set_bg_color(&stylePanel, COLOR_PANEL);
    lv_style_set_bg_opa(&stylePanel, LV_OPA_COVER);
    lv_style_set_border_color(&stylePanel, COLOR_BORDER);
    lv_style_set_border_width(&stylePanel, 1);
    lv_style_set_radius(&stylePanel, 12);
    lv_style_set_pad_all(&stylePanel, 12);
    
    lv_style_init(&styleCard);
    lv_style_set_bg_color(&styleCard, COLOR_PANEL);
    lv_style_set_bg_opa(&styleCard, LV_OPA_COVER);
    lv_style_set_border_color(&styleCard, COLOR_BORDER);
    lv_style_set_border_width(&styleCard, 1);
    lv_style_set_radius(&styleCard, 8);
    lv_style_set_pad_all(&styleCard, 8);
    
    lv_style_init(&styleBtn);
    lv_style_set_bg_color(&styleBtn, COLOR_ACCENT);
    lv_style_set_bg_opa(&styleBtn, LV_OPA_COVER);
    lv_style_set_radius(&styleBtn, 8);
    lv_style_set_pad_hor(&styleBtn, 16);
    lv_style_set_pad_ver(&styleBtn, 8);
}

}
