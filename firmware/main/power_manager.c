#include "power_manager.h"

#include "esp_log.h"
#include "esp_sleep.h"
#include "freertos/FreeRTOS.h"
#include "freertos/timers.h"
#include <time.h>

static const char *TAG = "power_manager";

typedef struct {
    power_manager_config_t config;
    TimerHandle_t timer;
    TickType_t last_activity;
    power_display_state_t display_state;
    bool auto_dim_enabled;
    bool deep_sleep_enabled;
} power_manager_ctx_t;

static power_manager_ctx_t s_ctx = {0};

static bool is_night_time(int start_hour, int end_hour)
{
    time_t now = time(NULL);
    struct tm info = {0};
    localtime_r(&now, &info);

    if (start_hour == end_hour) {
        return false;
    }

    if (start_hour < end_hour) {
        return info.tm_hour >= start_hour && info.tm_hour < end_hour;
    }

    return info.tm_hour >= start_hour || info.tm_hour < end_hour;
}

static void update_display_state(power_display_state_t next_state)
{
    if (s_ctx.display_state == next_state) {
        return;
    }

    s_ctx.display_state = next_state;
    if (s_ctx.config.display_cb) {
        s_ctx.config.display_cb(next_state, s_ctx.config.cb_ctx);
    }
}

static void power_manager_timer_cb(TimerHandle_t timer)
{
    (void)timer;

    TickType_t now = xTaskGetTickCount();
    uint32_t idle_ms = (uint32_t)((now - s_ctx.last_activity) * portTICK_PERIOD_MS);

    uint32_t dim_ms = s_ctx.config.dim_timeout_ms;
    uint32_t blank_ms = s_ctx.config.blank_timeout_ms;

    bool night = is_night_time(s_ctx.config.night_start_hour, s_ctx.config.night_end_hour);
    if (night) {
        dim_ms /= 2;
        blank_ms /= 2;
    }

    if (dim_ms == 0) {
        dim_ms = 1;
    }

    if (blank_ms <= dim_ms) {
        blank_ms = dim_ms + 1000;
    }

    if (s_ctx.auto_dim_enabled) {
        if (idle_ms >= blank_ms) {
            update_display_state(POWER_DISPLAY_OFF);
        } else if (idle_ms >= dim_ms) {
            update_display_state(POWER_DISPLAY_DIMMED);
        } else {
            update_display_state(POWER_DISPLAY_ACTIVE);
        }
    } else {
        update_display_state(POWER_DISPLAY_ACTIVE);
    }

    if (s_ctx.deep_sleep_enabled && s_ctx.config.deep_sleep_timeout_ms > 0 &&
        s_ctx.display_state == POWER_DISPLAY_OFF && idle_ms >= s_ctx.config.deep_sleep_timeout_ms && night) {
        ESP_LOGW(TAG, "Entering deep sleep after %ums of inactivity", idle_ms);
        esp_sleep_enable_timer_wakeup((uint64_t)s_ctx.config.deep_sleep_timeout_ms * 1000ULL);
        esp_deep_sleep_start();
    }
}

esp_err_t power_manager_init(const power_manager_config_t *config)
{
    if (!config || !config->display_cb || config->dim_timeout_ms == 0 || config->blank_timeout_ms == 0) {
        return ESP_ERR_INVALID_ARG;
    }

    s_ctx.config = *config;
    s_ctx.last_activity = xTaskGetTickCount();
    s_ctx.display_state = POWER_DISPLAY_ACTIVE;
    s_ctx.auto_dim_enabled = config->auto_dim_enabled;
    s_ctx.deep_sleep_enabled = config->deep_sleep_enabled;

    s_ctx.timer = xTimerCreate("power_timer", pdMS_TO_TICKS(1000), pdTRUE, NULL, power_manager_timer_cb);
    if (!s_ctx.timer) {
        return ESP_ERR_NO_MEM;
    }

    if (xTimerStart(s_ctx.timer, 0) != pdPASS) {
        return ESP_FAIL;
    }

    ESP_LOGI(TAG, "Power manager initialized: dim in %ums, blank in %ums", config->dim_timeout_ms, config->blank_timeout_ms);
    return ESP_OK;
}

void power_manager_mark_activity(void)
{
    s_ctx.last_activity = xTaskGetTickCount();
    update_display_state(POWER_DISPLAY_ACTIVE);
}

void power_manager_handle_touch(void)
{
    ESP_LOGD(TAG, "Touch activity detected");
    power_manager_mark_activity();
}

void power_manager_handle_rtc_alarm(void)
{
    ESP_LOGI(TAG, "RTC alarm woke the device");
    power_manager_mark_activity();
}

void power_manager_set_auto_dim_enabled(bool enabled)
{
    s_ctx.auto_dim_enabled = enabled;
    power_manager_mark_activity();
    ESP_LOGI(TAG, "Auto-dim %s", enabled ? "enabled" : "disabled");
}

void power_manager_set_deep_sleep_enabled(bool enabled)
{
    s_ctx.deep_sleep_enabled = enabled;
    power_manager_mark_activity();
    ESP_LOGI(TAG, "Deep sleep %s", enabled ? "enabled" : "disabled");
}

bool power_manager_is_auto_dim_enabled(void)
{
    return s_ctx.auto_dim_enabled;
}

bool power_manager_is_deep_sleep_enabled(void)
{
    return s_ctx.deep_sleep_enabled;
}

