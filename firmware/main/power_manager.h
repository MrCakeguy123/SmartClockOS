#pragma once

#include "esp_err.h"
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    POWER_DISPLAY_ACTIVE = 0,
    POWER_DISPLAY_DIMMED,
    POWER_DISPLAY_OFF,
} power_display_state_t;

typedef void (*power_display_cb_t)(power_display_state_t state, void *ctx);

typedef struct {
    uint32_t dim_timeout_ms;
    uint32_t blank_timeout_ms;
    uint32_t deep_sleep_timeout_ms;
    int night_start_hour;
    int night_end_hour;
    bool auto_dim_enabled;
    bool deep_sleep_enabled;
    power_display_cb_t display_cb;
    void *cb_ctx;
} power_manager_config_t;

esp_err_t power_manager_init(const power_manager_config_t *config);
void power_manager_mark_activity(void);
void power_manager_handle_touch(void);
void power_manager_handle_rtc_alarm(void);
void power_manager_set_auto_dim_enabled(bool enabled);
void power_manager_set_deep_sleep_enabled(bool enabled);
bool power_manager_is_auto_dim_enabled(void);
bool power_manager_is_deep_sleep_enabled(void);

#ifdef __cplusplus
}
#endif

