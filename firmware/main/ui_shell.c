#include "ui_shell.h"
#include "config.h"

#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "lvgl.h"
#include <stdbool.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <time.h>
#include "version.h"
#include "weather_service.h"

static const char *TAG = "ui_shell";

// Get weather icon symbol based on weather code
// Uses Unicode weather symbols that work with standard fonts
static const char *get_weather_icon(int code)
{
    switch (code) {
    case 0:
        return "\xE2\x98\x80"; // ☀ Sun
    case 1:
    case 2:
        return "\xE2\x9B\x85"; // ⛅ Partly cloudy
    case 3:
        return "\xE2\x98\x81"; // ☁ Cloud
    case 45:
    case 48:
        return "\xF0\x9F\x8C\xAB"; // 🌫 Fog
    case 51:
    case 53:
    case 55:
    case 56:
    case 57:
    case 61:
    case 63:
    case 65:
    case 66:
    case 67:
    case 80:
    case 81:
    case 82:
        return "\xE2\x98\x94"; // ☔ Rain
    case 71:
    case 73:
    case 75:
    case 77:
    case 85:
    case 86:
        return "\xE2\x9D\x84"; // ❄ Snow
    case 95:
    case 96:
    case 99:
        return "\xE2\x9A\xA1"; // ⚡ Thunderstorm
    default:
        return "?";
    }
}

typedef struct {
    lv_obj_t *loading_title;
    lv_obj_t *loading_status;
    lv_obj_t *loading_bar;
    lv_obj_t *time_label;
    lv_obj_t *sub_label;
    lv_obj_t *weather_label;
    lv_obj_t *weather_icon_label;
    lv_obj_t *weather_details_label;
    lv_obj_t *sun_label;
    lv_obj_t *brand_label;
    lv_obj_t *version_label;
    lv_obj_t *status_box;
    lv_obj_t *status_title;
    lv_obj_t *status_subtitle;
    lv_obj_t *brightness_overlay;
    lv_obj_t *settings_panel;
    lv_obj_t *auto_dim_switch;
    lv_obj_t *deep_sleep_switch;
    bool updating_toggles;
    int weather_ticks;
    bool clock_ready;
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

static void ui_shell_apply_brightness(ui_shell_ctx_t *ctx, ui_brightness_state_t state)
{
    if (!ctx->brightness_overlay) {
        return;
    }

    lv_opa_t overlay_opa = LV_OPA_TRANSP;
    lv_opa_t text_opa = LV_OPA_COVER;

    switch (state) {
        case UI_BRIGHTNESS_ACTIVE:
            overlay_opa = LV_OPA_TRANSP;
            text_opa = LV_OPA_COVER;
            break;
        case UI_BRIGHTNESS_DIMMED:
            overlay_opa = LV_OPA_50;
            text_opa = LV_OPA_80;
            break;
        case UI_BRIGHTNESS_OFF:
            overlay_opa = LV_OPA_80;
            text_opa = LV_OPA_60;
            break;
        default:
            break;
    }

    lv_obj_set_style_bg_opa(ctx->brightness_overlay, overlay_opa, 0);

    lv_obj_set_style_text_opa(ctx->time_label, text_opa, 0);
    lv_obj_set_style_text_opa(ctx->sub_label, text_opa, 0);
    lv_obj_set_style_text_opa(ctx->weather_label, text_opa, 0);
    if (ctx->weather_icon_label) {
        lv_obj_set_style_text_opa(ctx->weather_icon_label, text_opa, 0);
    }
    if (ctx->weather_details_label) {
        lv_obj_set_style_text_opa(ctx->weather_details_label, text_opa, 0);
    }
    if (ctx->sun_label) {
        lv_obj_set_style_text_opa(ctx->sun_label, text_opa, 0);
    }
    lv_obj_set_style_text_opa(ctx->brand_label, text_opa, 0);
    lv_obj_set_style_text_opa(ctx->version_label, text_opa, 0);
    lv_obj_set_style_text_opa(ctx->status_title, text_opa, 0);
    lv_obj_set_style_text_opa(ctx->status_subtitle, text_opa, 0);
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
    strlcat(sub_buf, " • " LOCATION_NAME, sizeof(sub_buf));
    lv_label_set_text(ctx->sub_label, sub_buf);

    ctx->weather_ticks++;
    if (ctx->weather_ticks >= 300 && ctx->config.weather_request_cb) {
        ctx->config.weather_request_cb(ctx->config.weather_request_ctx);
        ctx->weather_ticks = 0;
    }
}

static void settings_switch_handler(lv_event_t *e)
{
    ui_shell_ctx_t *ctx = (ui_shell_ctx_t *)lv_event_get_user_data(e);
    if (!ctx || ctx->updating_toggles) {
        return;
    }

    lv_obj_t *target = lv_event_get_target(e);
    const char *id = NULL;

    if (target == ctx->auto_dim_switch) {
        id = "auto_dim";
    } else if (target == ctx->deep_sleep_switch) {
        id = "deep_sleep";
    }

    if (!id) {
        return;
    }

    bool enabled = lv_obj_has_state(target, LV_STATE_CHECKED);
    if (ctx->config.settings_toggle_cb) {
        ctx->config.settings_toggle_cb(id, enabled, ctx->config.settings_toggle_ctx);
    }
}

static void ui_shell_create_settings_panel(ui_shell_ctx_t *ctx)
{
    lv_obj_t *panel = lv_obj_create(lv_scr_act());
    lv_obj_set_size(panel, 230, 120);
    lv_obj_align(panel, LV_ALIGN_BOTTOM_RIGHT, -8, -8);
    lv_obj_set_style_bg_color(panel, lv_color_hex(0x192532), 0);
    lv_obj_set_style_bg_opa(panel, LV_OPA_90, 0);
    lv_obj_set_style_radius(panel, 8, 0);
    lv_obj_set_style_border_width(panel, 0, 0);
    lv_obj_set_flex_flow(panel, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_style_pad_all(panel, 8, 0);
    lv_obj_set_style_pad_row(panel, 6, 0);

    lv_obj_t *title = lv_label_create(panel);
    lv_obj_set_style_text_font(title, &lv_font_montserrat_18, 0);
    lv_label_set_text(title, "Settings");

    lv_obj_t *row_dim = lv_obj_create(panel);
    lv_obj_set_style_bg_opa(row_dim, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_width(row_dim, 0, 0);
    lv_obj_set_style_pad_all(row_dim, 0, 0);
    lv_obj_set_flex_flow(row_dim, LV_FLEX_FLOW_ROW);
    lv_obj_set_width(row_dim, lv_pct(100));
    lv_obj_set_style_pad_column(row_dim, 8, 0);

    lv_obj_t *dim_label = lv_label_create(row_dim);
    lv_obj_set_style_text_font(dim_label, &lv_font_montserrat_14, 0);
    lv_label_set_text(dim_label, "Auto dim");

    lv_obj_t *dim_switch = lv_switch_create(row_dim);
    lv_obj_add_state(dim_switch, LV_STATE_CHECKED);
    lv_obj_add_event_cb(dim_switch, settings_switch_handler, LV_EVENT_VALUE_CHANGED, ctx);

    lv_obj_t *row_sleep = lv_obj_create(panel);
    lv_obj_set_style_bg_opa(row_sleep, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_width(row_sleep, 0, 0);
    lv_obj_set_style_pad_all(row_sleep, 0, 0);
    lv_obj_set_flex_flow(row_sleep, LV_FLEX_FLOW_ROW);
    lv_obj_set_width(row_sleep, lv_pct(100));
    lv_obj_set_style_pad_column(row_sleep, 8, 0);

    lv_obj_t *sleep_label = lv_label_create(row_sleep);
    lv_obj_set_style_text_font(sleep_label, &lv_font_montserrat_14, 0);
    lv_label_set_text(sleep_label, "Deep sleep");

    lv_obj_t *sleep_switch = lv_switch_create(row_sleep);
    lv_obj_add_state(sleep_switch, LV_STATE_CHECKED);
    lv_obj_add_event_cb(sleep_switch, settings_switch_handler, LV_EVENT_VALUE_CHANGED, ctx);

    ctx->settings_panel = panel;
    ctx->auto_dim_switch = dim_switch;
    ctx->deep_sleep_switch = sleep_switch;
    ctx->updating_toggles = false;
}

static void ui_shell_create_loading_ui(ui_shell_ctx_t *ctx)
{
    lv_obj_t *screen = lv_scr_act();

    // Background gradient
    static lv_style_t bg_style;
    lv_style_init(&bg_style);
    lv_style_set_bg_color(&bg_style, lv_color_hex(0x102030));
    lv_style_set_bg_grad_color(&bg_style, lv_color_hex(0x203040));
    lv_style_set_bg_grad_dir(&bg_style, LV_GRAD_DIR_VER);
    lv_obj_add_style(screen, &bg_style, 0);

    lv_obj_t *title = lv_label_create(screen);
    lv_obj_set_style_text_font(title, &lv_font_montserrat_34, 0);
    lv_label_set_text(title, "SmartClock OS");
    lv_obj_align(title, LV_ALIGN_CENTER, 0, -20);

    lv_obj_t *status = lv_label_create(screen);
    lv_obj_set_style_text_font(status, &lv_font_montserrat_18, 0);
    lv_label_set_text(status, "Loading...");
    lv_obj_align(status, LV_ALIGN_CENTER, 0, 12);

    lv_obj_t *bar = lv_bar_create(screen);
    lv_obj_set_size(bar, 200, 10);
    lv_bar_set_range(bar, 0, 100);
    lv_bar_set_value(bar, 0, LV_ANIM_OFF);
    lv_obj_align(bar, LV_ALIGN_CENTER, 0, 40);

    ctx->loading_title = title;
    ctx->loading_status = status;
    ctx->loading_bar = bar;
    ctx->clock_ready = false;
}

static void ui_shell_create_clock_ui(ui_shell_ctx_t *ctx)
{
    if (ctx->loading_title) {
        lv_obj_del(ctx->loading_title);
        ctx->loading_title = NULL;
    }
    if (ctx->loading_status) {
        lv_obj_del(ctx->loading_status);
        ctx->loading_status = NULL;
    }
    if (ctx->loading_bar) {
        lv_obj_del(ctx->loading_bar);
        ctx->loading_bar = NULL;
    }

    lv_obj_t *screen = lv_scr_act();

    // Background gradient
    static lv_style_t bg_style;
    lv_style_init(&bg_style);
    lv_style_set_bg_color(&bg_style, lv_color_hex(0x102030));
    lv_style_set_bg_grad_color(&bg_style, lv_color_hex(0x203040));
    lv_style_set_bg_grad_dir(&bg_style, LV_GRAD_DIR_VER);
    lv_obj_add_style(screen, &bg_style, 0);

    // Branding
    lv_obj_t *brand_label = lv_label_create(screen);
    lv_obj_set_style_text_font(brand_label, &lv_font_montserrat_18, 0);
    lv_obj_set_style_text_color(brand_label, lv_color_white(), 0);
    lv_label_set_text(brand_label, "SmartClock OS");
    lv_obj_align(brand_label, LV_ALIGN_TOP_LEFT, 12, 10);

    lv_obj_t *version_label = lv_label_create(screen);
    lv_obj_set_style_text_font(version_label, &lv_font_montserrat_14, 0);
    lv_obj_set_style_text_color(version_label, lv_color_hex(0xaec0d6), 0);
    lv_label_set_text_fmt(version_label, "Version %s", SMARTCLOCK_OS_VERSION);
    lv_obj_align_to(version_label, brand_label, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 2);

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
    lv_obj_align(sub_label, LV_ALIGN_CENTER, 0, 50);

    // Weather card (bottom of screen)
    lv_obj_t *weather_card = lv_obj_create(screen);
    lv_obj_set_size(weather_card, 450, 95);
    lv_obj_align(weather_card, LV_ALIGN_BOTTOM_MID, 0, -10);
    lv_obj_set_style_bg_color(weather_card, lv_color_hex(0x152238), 0);
    lv_obj_set_style_border_color(weather_card, lv_color_hex(0x2a4060), 0);
    lv_obj_set_style_border_width(weather_card, 1, 0);
    lv_obj_set_style_radius(weather_card, 12, 0);
    lv_obj_clear_flag(weather_card, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_style_pad_all(weather_card, 10, 0);

    // Weather icon (left side)
    lv_obj_t *weather_icon_label = lv_label_create(weather_card);
    lv_obj_set_style_text_font(weather_icon_label, &lv_font_montserrat_28, 0);
    lv_obj_set_style_text_color(weather_icon_label, lv_color_hex(0xffcc00), 0);
    lv_label_set_text(weather_icon_label, "...");
    lv_obj_align(weather_icon_label, LV_ALIGN_LEFT_MID, 5, 0);

    // Main weather label (temperature + condition)
    lv_obj_t *weather_label = lv_label_create(weather_card);
    lv_obj_set_style_text_font(weather_label, &lv_font_montserrat_26, 0);
    lv_obj_set_style_text_color(weather_label, lv_color_white(), 0);
    lv_label_set_text(weather_label, "Loading...");
    lv_obj_align(weather_label, LV_ALIGN_LEFT_MID, 50, -12);

    // Weather details (high/low/feels like)
    lv_obj_t *weather_details_label = lv_label_create(weather_card);
    lv_obj_set_style_text_font(weather_details_label, &lv_font_montserrat_14, 0);
    lv_obj_set_style_text_color(weather_details_label, lv_color_hex(0x7eb8da), 0);
    lv_label_set_text(weather_details_label, "");
    lv_obj_align(weather_details_label, LV_ALIGN_LEFT_MID, 50, 15);

    // Sunrise/Sunset (right side)
    lv_obj_t *sun_label = lv_label_create(weather_card);
    lv_obj_set_style_text_font(sun_label, &lv_font_montserrat_14, 0);
    lv_obj_set_style_text_color(sun_label, lv_color_hex(0xffcc00), 0);
    lv_label_set_text(sun_label, "");
    lv_obj_align(sun_label, LV_ALIGN_RIGHT_MID, -10, 0);

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

    lv_obj_t *overlay = lv_obj_create(screen);
    lv_obj_set_size(overlay, LV_HOR_RES, LV_VER_RES);
    lv_obj_set_style_bg_color(overlay, lv_color_black(), 0);
    lv_obj_set_style_bg_opa(overlay, LV_OPA_TRANSP, 0);
    lv_obj_clear_flag(overlay, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_add_flag(overlay, LV_OBJ_FLAG_EVENT_BUBBLE);
    lv_obj_clear_flag(overlay, LV_OBJ_FLAG_CLICKABLE);

    ui_shell_create_settings_panel(ctx);

    ctx->time_label = time_label;
    ctx->sub_label = sub_label;
    ctx->weather_label = weather_label;
    ctx->weather_icon_label = weather_icon_label;
    ctx->weather_details_label = weather_details_label;
    ctx->sun_label = sun_label;
    ctx->brand_label = brand_label;
    ctx->version_label = version_label;
    ctx->status_box = status_box;
    ctx->status_title = status_title;
    ctx->status_subtitle = status_subtitle;
    ctx->brightness_overlay = overlay;
    ctx->weather_ticks = 300; // force immediate first refresh
    ctx->clock_ready = true;

    ui_shell_apply_brightness(ctx, UI_BRIGHTNESS_ACTIVE);

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

    ui_shell_create_loading_ui(&s_ctx);

    ESP_LOGI(TAG, "UI shell initialized");
    return ESP_OK;
}

void ui_shell_update_boot_status(const char *module_name, uint8_t percent)
{
    if (!s_ctx.loading_status || !s_ctx.loading_bar) {
        return;
    }

    char status_text[64];
    if (module_name && module_name[0] != '\0') {
        snprintf(status_text, sizeof(status_text), "Loading %s", module_name);
    } else {
        strncpy(status_text, "Loading...", sizeof(status_text));
        status_text[sizeof(status_text) - 1] = '\0';
    }

    lv_label_set_text(s_ctx.loading_status, status_text);
    lv_bar_set_value(s_ctx.loading_bar, percent, LV_ANIM_OFF);

    if (percent >= 100 && !s_ctx.clock_ready) {
        s_ctx.clock_ready = true;
        ui_shell_create_clock_ui(&s_ctx);
    }
}

void ui_shell_update_weather(const char *text)
{
    if (!text || !s_ctx.weather_label) {
        return;
    }
    lv_label_set_text(s_ctx.weather_label, text);
}

void ui_shell_update_weather_data(const weather_data_t *data)
{
    if (!data) {
        return;
    }

    // Update main weather label (temp + condition)
    if (s_ctx.weather_label) {
        char buf[64];
        snprintf(buf, sizeof(buf), "%.0f\xC2\xB0F • %s", data->temp_f, data->condition);
        lv_label_set_text(s_ctx.weather_label, buf);
    }

    // Update weather icon
    if (s_ctx.weather_icon_label) {
        lv_label_set_text(s_ctx.weather_icon_label, get_weather_icon(data->weather_code));
    }

    // Update weather details (high/low/feels like)
    if (s_ctx.weather_details_label) {
        char details[64];
        snprintf(details, sizeof(details), "H:%.0f\xC2\xB0 L:%.0f\xC2\xB0 • Feels %.0f\xC2\xB0",
                 data->high_f, data->low_f, data->feels_like_f);
        lv_label_set_text(s_ctx.weather_details_label, details);
    }

    // Update sunrise/sunset
    if (s_ctx.sun_label) {
        char sun[64];
        snprintf(sun, sizeof(sun), "Rise: %s\nSet: %s", data->sunrise, data->sunset);
        lv_label_set_text(s_ctx.sun_label, sun);
    }
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

void ui_shell_set_brightness_state(ui_brightness_state_t state)
{
    ui_shell_apply_brightness(&s_ctx, state);
}

void ui_shell_update_power_quick_toggles(bool auto_dim_enabled, bool deep_sleep_enabled)
{
    if (!s_ctx.auto_dim_switch || !s_ctx.deep_sleep_switch) {
        return;
    }

    s_ctx.updating_toggles = true;

    if (auto_dim_enabled) {
        lv_obj_add_state(s_ctx.auto_dim_switch, LV_STATE_CHECKED);
    } else {
        lv_obj_clear_state(s_ctx.auto_dim_switch, LV_STATE_CHECKED);
    }

    if (deep_sleep_enabled) {
        lv_obj_add_state(s_ctx.deep_sleep_switch, LV_STATE_CHECKED);
    } else {
        lv_obj_clear_state(s_ctx.deep_sleep_switch, LV_STATE_CHECKED);
    }

    s_ctx.updating_toggles = false;
}

