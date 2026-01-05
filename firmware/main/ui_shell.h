#pragma once

#include "esp_err.h"
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef void (*ui_weather_request_cb_t)(void *ctx);

typedef struct {
    ui_weather_request_cb_t weather_request_cb;
    void *weather_request_ctx;
} ui_shell_config_t;

esp_err_t ui_shell_init(const ui_shell_config_t *config);
void ui_shell_update_weather(const char *text);
void ui_shell_show_onboarding(const char *primary, const char *secondary);

#ifdef __cplusplus
}
#endif

