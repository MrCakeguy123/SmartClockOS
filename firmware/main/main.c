#include "esp_event.h"
#include "esp_log.h"
#include "esp_netif.h"
#include "nvs_flash.h"

#include "provisioning_manager.h"
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

void app_main(void)
{
    ESP_LOGI(TAG, "booting SmartClockOS");

    ESP_ERROR_CHECK(app_init_nvs());
    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());

    time_service_config_t time_cfg = {
        .server = "pool.ntp.org",
        .sync_cb = on_time_synced,
        .cb_ctx = NULL,
    };

    weather_service_config_t weather_cfg = {
        .update_cb = on_weather_updated,
        .cb_ctx = NULL,
    };

    ui_shell_config_t ui_cfg = {
        .weather_request_cb = on_weather_requested,
        .weather_request_ctx = NULL,
    };

    ESP_ERROR_CHECK(time_service_init(&time_cfg));
    ESP_ERROR_CHECK(weather_service_init(&weather_cfg));
    ESP_ERROR_CHECK(ui_shell_init(&ui_cfg));

    provisioning_manager_config_t prov_cfg = {
        .state_cb = on_network_event,
        .cb_ctx = NULL,
    };
    ESP_ERROR_CHECK(provisioning_manager_init(&prov_cfg));
}

