#pragma once

#include "esp_err.h"
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

// Forward declaration
typedef struct weather_data_t weather_data_t;

typedef void (*ui_weather_request_cb_t)(void *ctx);
typedef void (*ui_settings_toggle_cb_t)(const char *toggle_id, bool enabled, void *ctx);

typedef enum {
    UI_BRIGHTNESS_ACTIVE = 0,
    UI_BRIGHTNESS_DIMMED,
    UI_BRIGHTNESS_OFF,
} ui_brightness_state_t;

typedef struct {
    ui_weather_request_cb_t weather_request_cb;
    void *weather_request_ctx;
    ui_settings_toggle_cb_t settings_toggle_cb;
    void *settings_toggle_ctx;
} ui_shell_config_t;

esp_err_t ui_shell_init(const ui_shell_config_t *config);
void ui_shell_update_weather(const char *text);
void ui_shell_update_weather_data(const weather_data_t *data);
void ui_shell_show_onboarding(const char *primary, const char *secondary);
void ui_shell_set_brightness_state(ui_brightness_state_t state);
void ui_shell_update_power_quick_toggles(bool auto_dim_enabled, bool deep_sleep_enabled);
void ui_shell_update_boot_status(const char *module_name, uint8_t percent);

#ifdef __cplusplus
}
#endif

