#include "time_service.h"

#include "esp_log.h"
#include "esp_netif_sntp.h"
#include "esp_sntp.h"
#include <stdbool.h>
#include <sys/time.h>

static const char *TAG = "time_service";

static time_service_config_t s_config = {0};
static bool s_started = false;

static void time_sync_notification(struct timeval *tv)
{
    (void)tv;
    ESP_LOGI(TAG, "Time synchronized");
    if (s_config.sync_cb) {
        s_config.sync_cb(s_config.cb_ctx);
    }
}

esp_err_t time_service_init(const time_service_config_t *config)
{
    if (!config || !config->server) {
        return ESP_ERR_INVALID_ARG;
    }

    s_config = *config;

    esp_sntp_config_t sntp_config = ESP_NETIF_SNTP_DEFAULT_CONFIG(config->server);
    sntp_config.sync_cb = &time_sync_notification;
    esp_netif_sntp_init(&sntp_config);

    return ESP_OK;
}

esp_err_t time_service_start(void)
{
    if (s_started) {
        return ESP_OK;
    }

    esp_err_t err = esp_netif_sntp_start();
    if (err == ESP_OK) {
        s_started = true;
        ESP_LOGI(TAG, "SNTP started");
    }
    return err;
}

