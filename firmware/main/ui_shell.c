#include "ui_shell.h"

#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "lvgl.h"
#include <string.h>
#include <time.h>

static const char *TAG = "ui_shell";

typedef struct {
    lv_obj_t *time_label;
    lv_obj_t *sub_label;
    lv_obj_t *weather_label;
    lv_obj_t *status_box;
    lv_obj_t *status_title;
    lv_obj_t *status_subtitle;
    int weather_ticks;
    ui_shell_config_t config;
} ui_shell_ctx_t;

static ui_shell_ctx_t s_ctx = {0};

static void ui_shell_lvgl_tick(void *arg)
{
    (void)arg;
    const TickType_t delay = pdMS_TO_TICKS(1);
    while (true) {
        lv_tick_inc(1);
        vTaskDelay(delay);
    }
}

static void ui_shell_lvgl_loop(void *arg)
{
    (void)arg;
    const TickType_t delay = pdMS_TO_TICKS(5);
    while (true) {
        lv_task_handler();
        vTaskDelay(delay);
    }
}

static void ui_shell_update_clock(lv_timer_t *timer)
{
    ui_shell_ctx_t *ctx = (ui_shell_ctx_t *)timer->user_data;

    time_t now = time(NULL);
    struct tm info = {0};
    localtime_r(&now, &info);

    char time_buf[8];
    strftime(time_buf, sizeof(time_buf), "%H:%M", &info);
    lv_label_set_text(ctx->time_label, time_buf);

    char sub_buf[32];
    strftime(sub_buf, sizeof(sub_buf), "%a", &info);
    strlcat(sub_buf, " • Paris", sizeof(sub_buf));
    lv_label_set_text(ctx->sub_label, sub_buf);

    ctx->weather_ticks++;
    if (ctx->weather_ticks >= 300 && ctx->config.weather_request_cb) {
        ctx->config.weather_request_cb(ctx->config.weather_request_ctx);
        ctx->weather_ticks = 0;
    }
}

static void ui_shell_create_clock_ui(ui_shell_ctx_t *ctx)
{
    lv_obj_t *screen = lv_scr_act();

    // Background gradient
    static lv_style_t bg_style;
    lv_style_init(&bg_style);
    lv_style_set_bg_color(&bg_style, lv_color_hex(0x102030));
    lv_style_set_bg_grad_color(&bg_style, lv_color_hex(0x203040));
    lv_style_set_bg_grad_dir(&bg_style, LV_GRAD_DIR_VER);
    lv_obj_add_style(screen, &bg_style, 0);

    // Time label
    lv_obj_t *time_label = lv_label_create(screen);
    lv_obj_set_style_text_font(time_label, &lv_font_montserrat_48, 0);
    lv_obj_set_style_text_color(time_label, lv_color_white(), 0);
    lv_label_set_text(time_label, "00:00");
    lv_obj_center(time_label);

    // Seconds animation: subtle scale pulse
    lv_anim_t anim;
    lv_anim_init(&anim);
    lv_anim_set_var(&anim, time_label);
    lv_anim_set_values(&anim, 100, 103);
    lv_anim_set_exec_cb(&anim, (lv_anim_exec_xcb_t)lv_obj_set_style_transform_zoom);
    lv_anim_set_time(&anim, 1000);
    lv_anim_set_repeat_count(&anim, LV_ANIM_REPEAT_INFINITE);
    lv_anim_start(&anim);

    // Subtitle for date/location
    lv_obj_t *sub_label = lv_label_create(screen);
    lv_obj_set_style_text_font(sub_label, &lv_font_montserrat_18, 0);
    lv_label_set_text(sub_label, "-- • --");
    lv_obj_align(sub_label, LV_ALIGN_BOTTOM_MID, 0, -32);

    // Weather label
    lv_obj_t *weather_label = lv_label_create(screen);
    lv_obj_set_style_text_font(weather_label, &lv_font_montserrat_22, 0);
    lv_label_set_text(weather_label, "--°C • --");
    lv_obj_align(weather_label, LV_ALIGN_BOTTOM_MID, 0, -4);

    // Status panel for onboarding/provisioning
    lv_obj_t *status_box = lv_obj_create(screen);
    lv_obj_set_size(status_box, 220, 70);
    lv_obj_align(status_box, LV_ALIGN_TOP_MID, 0, 10);
    lv_obj_set_style_bg_color(status_box, lv_color_hex(0x243447), 0);
    lv_obj_set_style_bg_opa(status_box, LV_OPA_COVER, 0);
    lv_obj_set_style_radius(status_box, 8, 0);
    lv_obj_set_style_border_width(status_box, 0, 0);

    lv_obj_t *status_title = lv_label_create(status_box);
    lv_obj_set_style_text_font(status_title, &lv_font_montserrat_20, 0);
    lv_obj_set_style_text_color(status_title, lv_color_white(), 0);
    lv_label_set_text(status_title, "Connecting to Wi-Fi");
    lv_obj_align(status_title, LV_ALIGN_TOP_MID, 0, 6);

    lv_obj_t *status_subtitle = lv_label_create(status_box);
    lv_obj_set_style_text_font(status_subtitle, &lv_font_montserrat_14, 0);
    lv_obj_set_style_text_color(status_subtitle, lv_color_hex(0xaec0d6), 0);
    lv_label_set_text(status_subtitle, "Preparing network");
    lv_obj_align(status_subtitle, LV_ALIGN_BOTTOM_MID, 0, -6);

    ctx->time_label = time_label;
    ctx->sub_label = sub_label;
    ctx->weather_label = weather_label;
    ctx->status_box = status_box;
    ctx->status_title = status_title;
    ctx->status_subtitle = status_subtitle;
    ctx->weather_ticks = 300; // force immediate first refresh

    lv_timer_create(ui_shell_update_clock, 1000, ctx);
}

esp_err_t ui_shell_init(const ui_shell_config_t *config)
{
    if (!config) {
        return ESP_ERR_INVALID_ARG;
    }

    s_ctx.config = *config;

    lv_init();

    xTaskCreate(ui_shell_lvgl_tick, "lv_tick", 2048, NULL, 5, NULL);
    xTaskCreate(ui_shell_lvgl_loop, "lv_loop", 4096, NULL, 5, NULL);

    ui_shell_create_clock_ui(&s_ctx);

    ESP_LOGI(TAG, "UI shell initialized");
    return ESP_OK;
}

void ui_shell_update_weather(const char *text)
{
    if (!text || !s_ctx.weather_label) {
        return;
    }
    lv_label_set_text(s_ctx.weather_label, text);
}

void ui_shell_show_onboarding(const char *primary, const char *secondary)
{
    if (!s_ctx.status_box) {
        return;
    }

    if (primary) {
        lv_label_set_text(s_ctx.status_title, primary);
    }

    if (secondary) {
        lv_label_set_text(s_ctx.status_subtitle, secondary);
    }
}

