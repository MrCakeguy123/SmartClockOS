#include "lvgl.h"

#include <stdio.h>
#include <string.h>

lv_font_t lv_font_montserrat_14 = {0};
lv_font_t lv_font_montserrat_18 = {0};
lv_font_t lv_font_montserrat_20 = {0};
lv_font_t lv_font_montserrat_26 = {0};
lv_font_t lv_font_montserrat_28 = {0};
lv_font_t lv_font_montserrat_34 = {0};
lv_font_t lv_font_montserrat_48 = {0};

static lv_obj_t s_screen = {0};

void lv_init(void) {}

void lv_tick_inc(uint32_t ms)
{
    (void)ms;
}

void lv_task_handler(void) {}

lv_obj_t *lv_scr_act(void)
{
    return &s_screen;
}

static lv_obj_t *allocate_obj(lv_obj_t *parent)
{
    lv_obj_t *obj = calloc(1, sizeof(lv_obj_t));
    if (obj) {
        obj->parent = parent ? parent : &s_screen;
    }
    return obj;
}

lv_obj_t *lv_obj_create(lv_obj_t *parent)
{
    return allocate_obj(parent);
}

lv_obj_t *lv_label_create(lv_obj_t *parent)
{
    return allocate_obj(parent);
}

lv_obj_t *lv_bar_create(lv_obj_t *parent)
{
    return allocate_obj(parent);
}

lv_obj_t *lv_switch_create(lv_obj_t *parent)
{
    return allocate_obj(parent);
}

void lv_obj_del(lv_obj_t *obj)
{
    free(obj);
}

void lv_obj_set_size(lv_obj_t *obj, int32_t w, int32_t h)
{
    (void)obj;
    (void)w;
    (void)h;
}

void lv_obj_set_width(lv_obj_t *obj, int32_t w)
{
    (void)obj;
    (void)w;
}

void lv_obj_align(lv_obj_t *obj, lv_align_t align, int32_t x_ofs, int32_t y_ofs)
{
    if (!obj) {
        return;
    }
    obj->align = align;
    (void)x_ofs;
    (void)y_ofs;
}

void lv_obj_align_to(lv_obj_t *obj, const lv_obj_t *base, lv_align_t align, int32_t x_ofs, int32_t y_ofs)
{
    (void)base;
    lv_obj_align(obj, align, x_ofs, y_ofs);
}

void lv_obj_center(lv_obj_t *obj)
{
    lv_obj_align(obj, LV_ALIGN_CENTER, 0, 0);
}

void lv_obj_add_style(lv_obj_t *obj, const lv_style_t *style, uint32_t sel_part)
{
    (void)obj;
    (void)style;
    (void)sel_part;
}

void lv_obj_set_style_bg_color(lv_obj_t *obj, lv_color_t color, uint32_t sel_part)
{
    if (!obj) {
        return;
    }
    obj->color = color;
    (void)sel_part;
}

void lv_obj_set_style_bg_grad_color(lv_obj_t *obj, lv_color_t color, uint32_t sel_part)
{
    (void)obj;
    (void)color;
    (void)sel_part;
}

void lv_obj_set_style_bg_grad_dir(lv_obj_t *obj, lv_grad_dir_t dir, uint32_t sel_part)
{
    (void)obj;
    (void)dir;
    (void)sel_part;
}

void lv_obj_set_style_bg_opa(lv_obj_t *obj, lv_opa_t opa, uint32_t sel_part)
{
    if (!obj) {
        return;
    }
    obj->opa = opa;
    (void)sel_part;
}

void lv_obj_set_style_text_font(lv_obj_t *obj, const lv_font_t *font, uint32_t sel_part)
{
    (void)obj;
    (void)font;
    (void)sel_part;
}

void lv_obj_set_style_text_color(lv_obj_t *obj, lv_color_t color, uint32_t sel_part)
{
    if (!obj) {
        return;
    }
    obj->color = color;
    (void)sel_part;
}

void lv_obj_set_style_text_opa(lv_obj_t *obj, lv_opa_t opa, uint32_t sel_part)
{
    if (!obj) {
        return;
    }
    obj->opa = opa;
    (void)sel_part;
}

void lv_obj_set_style_radius(lv_obj_t *obj, int32_t radius, uint32_t sel_part)
{
    (void)obj;
    (void)radius;
    (void)sel_part;
}

void lv_obj_set_style_border_width(lv_obj_t *obj, int32_t width, uint32_t sel_part)
{
    (void)obj;
    (void)width;
    (void)sel_part;
}

void lv_obj_set_style_border_color(lv_obj_t *obj, lv_color_t color, uint32_t sel_part)
{
    (void)obj;
    (void)color;
    (void)sel_part;
}

void lv_obj_set_style_pad_all(lv_obj_t *obj, int32_t pad, uint32_t sel_part)
{
    (void)obj;
    (void)pad;
    (void)sel_part;
}

void lv_obj_set_style_pad_row(lv_obj_t *obj, int32_t pad, uint32_t sel_part)
{
    (void)obj;
    (void)pad;
    (void)sel_part;
}

void lv_obj_set_style_pad_column(lv_obj_t *obj, int32_t pad, uint32_t sel_part)
{
    (void)obj;
    (void)pad;
    (void)sel_part;
}

void lv_obj_set_style_transform_zoom(lv_obj_t *obj, int32_t zoom, uint32_t sel_part)
{
    (void)obj;
    (void)zoom;
    (void)sel_part;
}

void lv_obj_set_flex_flow(lv_obj_t *obj, lv_flex_flow_t flow)
{
    (void)obj;
    (void)flow;
}

void lv_obj_clear_flag(lv_obj_t *obj, lv_obj_flag_t flag)
{
    (void)obj;
    (void)flag;
}

void lv_obj_add_flag(lv_obj_t *obj, lv_obj_flag_t flag)
{
    (void)obj;
    (void)flag;
}

void lv_label_set_text(lv_obj_t *obj, const char *txt)
{
    (void)obj;
    (void)txt;
}

void lv_label_set_text_fmt(lv_obj_t *obj, const char *fmt, ...)
{
    (void)obj;
    (void)fmt;
}

void lv_bar_set_range(lv_obj_t *bar, int32_t min, int32_t max)
{
    (void)bar;
    (void)min;
    (void)max;
}

void lv_bar_set_value(lv_obj_t *bar, int32_t val, lv_anim_repeat_t anim)
{
    (void)bar;
    (void)val;
    (void)anim;
}

bool lv_obj_has_state(const lv_obj_t *obj, lv_state_t state)
{
    if (!obj) {
        return false;
    }
    return (obj->state & state) != 0;
}

void lv_obj_add_state(lv_obj_t *obj, lv_state_t state)
{
    if (!obj) {
        return;
    }
    obj->state |= state;
}

void lv_obj_clear_state(lv_obj_t *obj, lv_state_t state)
{
    if (!obj) {
        return;
    }
    obj->state &= ~state;
}

void lv_obj_add_event_cb(lv_obj_t *obj, void (*cb)(lv_event_t *), lv_event_code_t code, void *user_data)
{
    (void)obj;
    (void)cb;
    (void)code;
    (void)user_data;
}

void *lv_event_get_user_data(const lv_event_t *e)
{
    return e ? e->user_data : NULL;
}

lv_obj_t *lv_event_get_target(const lv_event_t *e)
{
    return e ? e->target : NULL;
}

lv_timer_t *lv_timer_create(lv_timer_cb_t cb, uint32_t period, void *user_data)
{
    lv_timer_t *t = calloc(1, sizeof(lv_timer_t));
    if (t) {
        t->cb = cb;
        t->period_ms = period;
        t->user_data = user_data;
    }
    return t;
}

void lv_style_init(lv_style_t *style)
{
    if (style) {
        memset(style, 0, sizeof(*style));
        style->initialized = 1;
    }
}

void lv_style_set_bg_color(lv_style_t *style, lv_color_t color)
{
    (void)style;
    (void)color;
}

void lv_style_set_bg_grad_color(lv_style_t *style, lv_color_t color)
{
    (void)style;
    (void)color;
}

void lv_style_set_bg_grad_dir(lv_style_t *style, lv_grad_dir_t dir)
{
    (void)style;
    (void)dir;
}

void lv_anim_init(lv_anim_t *a)
{
    if (a) {
        memset(a, 0, sizeof(*a));
    }
}

void lv_anim_set_var(lv_anim_t *a, void *var)
{
    if (a) {
        a->var = var;
    }
}

void lv_anim_set_values(lv_anim_t *a, int32_t start, int32_t end)
{
    if (a) {
        a->start = start;
        a->end = end;
    }
}

void lv_anim_set_exec_cb(lv_anim_t *a, lv_anim_exec_xcb_t exec_cb)
{
    if (a) {
        a->exec_cb = exec_cb;
    }
}

void lv_anim_set_time(lv_anim_t *a, uint32_t duration)
{
    (void)a;
    (void)duration;
}

void lv_anim_set_repeat_count(lv_anim_t *a, int32_t cnt)
{
    if (a) {
        a->repeat = cnt;
    }
}

void lv_anim_start(lv_anim_t *a)
{
    if (a && a->exec_cb) {
        a->exec_cb(a->var, a->start);
    }
}

