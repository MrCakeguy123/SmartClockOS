#pragma once

#include <stdbool.h>
#include <stdint.h>
#include <stdarg.h>
#include <stdlib.h>

#ifdef __cplusplus
extern "C" {
#endif

#ifndef CONFIG_LVGL_DISPLAY_H_RES
#define CONFIG_LVGL_DISPLAY_H_RES 480
#endif
#ifndef CONFIG_LVGL_DISPLAY_V_RES
#define CONFIG_LVGL_DISPLAY_V_RES 320
#endif
#ifndef CONFIG_LVGL_COLOR_DEPTH_16
#define CONFIG_LVGL_COLOR_DEPTH_16 1
#endif

#define LV_HOR_RES CONFIG_LVGL_DISPLAY_H_RES
#define LV_VER_RES CONFIG_LVGL_DISPLAY_V_RES

#define LV_OPA_TRANSP 0
#define LV_OPA_COVER 255
#define LV_OPA_50 128
#define LV_OPA_60 153
#define LV_OPA_80 204
#define LV_OPA_90 230

typedef uint8_t lv_opa_t;

typedef enum {
    LV_ALIGN_DEFAULT = 0,
    LV_ALIGN_TOP_LEFT,
    LV_ALIGN_TOP_MID,
    LV_ALIGN_TOP_RIGHT,
    LV_ALIGN_BOTTOM_LEFT,
    LV_ALIGN_BOTTOM_MID,
    LV_ALIGN_BOTTOM_RIGHT,
    LV_ALIGN_LEFT_MID,
    LV_ALIGN_RIGHT_MID,
    LV_ALIGN_CENTER,
    LV_ALIGN_OUT_TOP_LEFT,
    LV_ALIGN_OUT_TOP_MID,
    LV_ALIGN_OUT_TOP_RIGHT,
    LV_ALIGN_OUT_BOTTOM_LEFT,
    LV_ALIGN_OUT_BOTTOM_MID,
    LV_ALIGN_OUT_BOTTOM_RIGHT,
    LV_ALIGN_OUT_LEFT_TOP,
    LV_ALIGN_OUT_LEFT_MID,
    LV_ALIGN_OUT_LEFT_BOTTOM,
    LV_ALIGN_OUT_RIGHT_TOP,
    LV_ALIGN_OUT_RIGHT_MID,
    LV_ALIGN_OUT_RIGHT_BOTTOM,
} lv_align_t;

typedef enum {
    LV_GRAD_DIR_NONE = 0,
    LV_GRAD_DIR_VER,
} lv_grad_dir_t;

typedef enum {
    LV_OBJ_FLAG_SCROLLABLE = 1 << 0,
    LV_OBJ_FLAG_EVENT_BUBBLE = 1 << 1,
    LV_OBJ_FLAG_CLICKABLE = 1 << 2,
} lv_obj_flag_t;

typedef enum {
    LV_STATE_DEFAULT = 0,
    LV_STATE_CHECKED = 1 << 0,
} lv_state_t;

typedef enum {
    LV_EVENT_VALUE_CHANGED = 0,
} lv_event_code_t;

typedef enum {
    LV_ANIM_OFF = 0,
    LV_ANIM_REPEAT_INFINITE = -1,
} lv_anim_repeat_t;

typedef enum {
    LV_FLEX_FLOW_ROW = 0,
    LV_FLEX_FLOW_COLUMN,
} lv_flex_flow_t;

typedef struct {
    uint16_t full;
} lv_color_t;

static inline lv_color_t lv_color_hex(uint32_t hex)
{
    (void)hex;
    return (lv_color_t){0};
}

static inline lv_color_t lv_color_white(void)
{
    return (lv_color_t){0xffff};
}

static inline lv_color_t lv_color_black(void)
{
    return (lv_color_t){0};
}

typedef struct lv_font_t {
    uint8_t dummy;
} lv_font_t;

extern lv_font_t lv_font_montserrat_14;
extern lv_font_t lv_font_montserrat_18;
extern lv_font_t lv_font_montserrat_20;
extern lv_font_t lv_font_montserrat_26;
extern lv_font_t lv_font_montserrat_28;
extern lv_font_t lv_font_montserrat_34;
extern lv_font_t lv_font_montserrat_48;

typedef struct lv_obj_t {
    lv_state_t state;
    lv_align_t align;
    lv_color_t color;
    lv_opa_t opa;
    struct lv_obj_t *parent;
    void *user_data;
} lv_obj_t;

typedef struct lv_event_t {
    lv_obj_t *target;
    void *user_data;
} lv_event_t;

typedef struct lv_style_t {
    int initialized;
} lv_style_t;

typedef struct lv_timer_t lv_timer_t;

typedef void (*lv_timer_cb_t)(lv_timer_t *);

struct lv_timer_t {
    lv_timer_cb_t cb;
    void *user_data;
    uint32_t period_ms;
};

typedef void (*lv_anim_exec_xcb_t)(void *var, int32_t value);

typedef struct lv_anim_t {
    void *var;
    lv_anim_exec_xcb_t exec_cb;
    int32_t start;
    int32_t end;
    int32_t repeat;
} lv_anim_t;

void lv_init(void);
void lv_tick_inc(uint32_t ms);
void lv_task_handler(void);

lv_obj_t *lv_scr_act(void);

lv_obj_t *lv_obj_create(lv_obj_t *parent);
void lv_obj_del(lv_obj_t *obj);

void lv_obj_set_size(lv_obj_t *obj, int32_t w, int32_t h);
void lv_obj_set_width(lv_obj_t *obj, int32_t w);
void lv_obj_align(lv_obj_t *obj, lv_align_t align, int32_t x_ofs, int32_t y_ofs);
void lv_obj_align_to(lv_obj_t *obj, const lv_obj_t *base, lv_align_t align, int32_t x_ofs, int32_t y_ofs);
void lv_obj_center(lv_obj_t *obj);
void lv_obj_add_style(lv_obj_t *obj, const lv_style_t *style, uint32_t sel_part);
void lv_obj_set_style_bg_color(lv_obj_t *obj, lv_color_t color, uint32_t sel_part);
void lv_obj_set_style_bg_grad_color(lv_obj_t *obj, lv_color_t color, uint32_t sel_part);
void lv_obj_set_style_bg_grad_dir(lv_obj_t *obj, lv_grad_dir_t dir, uint32_t sel_part);
void lv_obj_set_style_bg_opa(lv_obj_t *obj, lv_opa_t opa, uint32_t sel_part);
void lv_obj_set_style_text_font(lv_obj_t *obj, const lv_font_t *font, uint32_t sel_part);
void lv_obj_set_style_text_color(lv_obj_t *obj, lv_color_t color, uint32_t sel_part);
void lv_obj_set_style_text_opa(lv_obj_t *obj, lv_opa_t opa, uint32_t sel_part);
void lv_obj_set_style_radius(lv_obj_t *obj, int32_t radius, uint32_t sel_part);
void lv_obj_set_style_border_width(lv_obj_t *obj, int32_t width, uint32_t sel_part);
void lv_obj_set_style_border_color(lv_obj_t *obj, lv_color_t color, uint32_t sel_part);
void lv_obj_set_style_pad_all(lv_obj_t *obj, int32_t pad, uint32_t sel_part);
void lv_obj_set_style_pad_row(lv_obj_t *obj, int32_t pad, uint32_t sel_part);
void lv_obj_set_style_pad_column(lv_obj_t *obj, int32_t pad, uint32_t sel_part);
void lv_obj_set_style_transform_zoom(lv_obj_t *obj, int32_t zoom, uint32_t sel_part);
void lv_obj_set_flex_flow(lv_obj_t *obj, lv_flex_flow_t flow);
void lv_obj_clear_flag(lv_obj_t *obj, lv_obj_flag_t flag);
void lv_obj_add_flag(lv_obj_t *obj, lv_obj_flag_t flag);

lv_obj_t *lv_label_create(lv_obj_t *parent);
void lv_label_set_text(lv_obj_t *obj, const char *txt);
void lv_label_set_text_fmt(lv_obj_t *obj, const char *fmt, ...);

lv_obj_t *lv_bar_create(lv_obj_t *parent);
void lv_bar_set_range(lv_obj_t *bar, int32_t min, int32_t max);
void lv_bar_set_value(lv_obj_t *bar, int32_t val, lv_anim_repeat_t anim);

lv_obj_t *lv_switch_create(lv_obj_t *parent);
bool lv_obj_has_state(const lv_obj_t *obj, lv_state_t state);
void lv_obj_add_state(lv_obj_t *obj, lv_state_t state);
void lv_obj_clear_state(lv_obj_t *obj, lv_state_t state);

void lv_obj_add_event_cb(lv_obj_t *obj, void (*cb)(lv_event_t *), lv_event_code_t code, void *user_data);
void *lv_event_get_user_data(const lv_event_t *e);
lv_obj_t *lv_event_get_target(const lv_event_t *e);

lv_timer_t *lv_timer_create(lv_timer_cb_t cb, uint32_t period, void *user_data);

void lv_style_init(lv_style_t *style);
void lv_style_set_bg_color(lv_style_t *style, lv_color_t color);
void lv_style_set_bg_grad_color(lv_style_t *style, lv_color_t color);
void lv_style_set_bg_grad_dir(lv_style_t *style, lv_grad_dir_t dir);

void lv_anim_init(lv_anim_t *a);
void lv_anim_set_var(lv_anim_t *a, void *var);
void lv_anim_set_values(lv_anim_t *a, int32_t start, int32_t end);
void lv_anim_set_exec_cb(lv_anim_t *a, lv_anim_exec_xcb_t exec_cb);
void lv_anim_set_time(lv_anim_t *a, uint32_t duration);
void lv_anim_set_repeat_count(lv_anim_t *a, int32_t cnt);
void lv_anim_start(lv_anim_t *a);

static inline int32_t lv_pct(int32_t percent)
{
    return percent;
}

#ifdef __cplusplus
}
#endif

