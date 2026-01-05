#include "esp_event.h"
#include "esp_log.h"
#include "esp_netif.h"
#include "nvs_flash.h"
#include <string.h>

#include "provisioning_manager.h"
#include "power_manager.h"
#include "time_service.h"
#include "ui_shell.h"
#include "weather_service.h"

static const char *TAG = "SmartClock";

static esp_err_t app_init_nvs(void)
{
    esp_err_t err = nvs_flash_init();
    if (err == ESP_ERR_NVS_NO_FREE_PAGES || err == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        err = nvs_flash_init();
    }
    return err;
}

static void on_time_synced(void *ctx)
{
    (void)ctx;
    ESP_LOGI(TAG, "Time sync callback fired");
    power_manager_handle_rtc_alarm();
}

static void on_weather_updated(const weather_data_t *data, void *ctx)
{
    (void)ctx;
    if (data) {
        ui_shell_update_weather(data->description);
    }
}

static void on_weather_requested(void *ctx)
{
    (void)ctx;
    power_manager_mark_activity();
    weather_service_request_update();
}

static void on_network_event(network_state_t state, void *ctx)
{
    (void)ctx;
    if (state == NETWORK_STATE_CONNECTED) {
        ESP_LOGI(TAG, "Network connected, starting SNTP");
        ESP_ERROR_CHECK(time_service_start());
        weather_service_request_update();
    }
}

static void on_display_power_state(power_display_state_t state, void *ctx)
{
    (void)ctx;
    switch (state) {
        case POWER_DISPLAY_ACTIVE:
            ui_shell_set_brightness_state(UI_BRIGHTNESS_ACTIVE);
            break;
        case POWER_DISPLAY_DIMMED:
            ui_shell_set_brightness_state(UI_BRIGHTNESS_DIMMED);
            break;
        case POWER_DISPLAY_OFF:
            ui_shell_set_brightness_state(UI_BRIGHTNESS_OFF);
            break;
        default:
            break;
    }
}

static void on_settings_toggle(const char *toggle_id, bool enabled, void *ctx)
{
    (void)ctx;
    power_manager_handle_touch();

    if (strcmp(toggle_id, "auto_dim") == 0) {
        power_manager_set_auto_dim_enabled(enabled);
    } else if (strcmp(toggle_id, "deep_sleep") == 0) {
        power_manager_set_deep_sleep_enabled(enabled);
    }

    ui_shell_update_power_quick_toggles(power_manager_is_auto_dim_enabled(), power_manager_is_deep_sleep_enabled());
}

void app_main(void)
{
    ESP_LOGI(TAG, "booting SmartClockOS");

    ESP_ERROR_CHECK(app_init_nvs());
    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());

    ui_shell_config_t ui_cfg = {
        .weather_request_cb = on_weather_requested,
        .weather_request_ctx = NULL,
        .settings_toggle_cb = on_settings_toggle,
        .settings_toggle_ctx = NULL,
    };

    ESP_ERROR_CHECK(ui_shell_init(&ui_cfg));
    ui_shell_update_boot_status("display", 10);

    time_service_config_t time_cfg = {
        .server = "pool.ntp.org",
        .sync_cb = on_time_synced,
        .cb_ctx = NULL,
    };

    weather_service_config_t weather_cfg = {
        .update_cb = on_weather_updated,
        .cb_ctx = NULL,
    };

    ui_shell_update_boot_status("time service", 25);
    ESP_ERROR_CHECK(time_service_init(&time_cfg));

    ui_shell_update_boot_status("weather service", 45);
    ESP_ERROR_CHECK(weather_service_init(&weather_cfg));

    power_manager_config_t power_cfg = {
        .dim_timeout_ms = 30000,
        .blank_timeout_ms = 60000,
        .deep_sleep_timeout_ms = 600000,
        .night_start_hour = 22,
        .night_end_hour = 6,
        .auto_dim_enabled = true,
        .deep_sleep_enabled = true,
        .display_cb = on_display_power_state,
        .cb_ctx = NULL,
    };
    ui_shell_update_boot_status("power manager", 65);
    ESP_ERROR_CHECK(power_manager_init(&power_cfg));
    ui_shell_update_power_quick_toggles(power_manager_is_auto_dim_enabled(), power_manager_is_deep_sleep_enabled());

    provisioning_manager_config_t prov_cfg = {
        .state_cb = on_network_event,
        .cb_ctx = NULL,
    };
    ui_shell_update_boot_status("provisioning", 80);
    ESP_ERROR_CHECK(provisioning_manager_init(&prov_cfg));

    ui_shell_update_boot_status("ready", 100);
}

