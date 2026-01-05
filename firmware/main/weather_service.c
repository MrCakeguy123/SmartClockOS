#include "weather_service.h"

#include "esp_log.h"
#include <stdio.h>
#include <string.h>

static const char *TAG = "weather_service";

static weather_service_config_t s_config = {0};

esp_err_t weather_service_init(const weather_service_config_t *config)
{
    if (!config || !config->update_cb) {
        return ESP_ERR_INVALID_ARG;
    }

    s_config = *config;
    ESP_LOGI(TAG, "Weather service initialized");
    return ESP_OK;
}

void weather_service_request_update(void)
{
    weather_data_t data = {0};
    // TODO: replace with HTTP client to a weather API using the device location.
    snprintf(data.description, sizeof(data.description), "22°C • Clear Skies");

    if (s_config.update_cb) {
        s_config.update_cb(&data, s_config.cb_ctx);
    }
}

