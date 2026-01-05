#include "network_manager.h"

#include "esp_event.h"
#include "esp_log.h"
#include "esp_wifi.h"
#include "esp_netif.h"
#include <string.h>

static const char *TAG = "network_manager";

static network_manager_config_t s_config = {0};
static bool s_connected = false;
static bool s_has_credentials = false;
static bool s_ap_running = false;
static char s_ssid[32] = {0};
static char s_password[64] = {0};

static void wifi_event_handler(void *arg, esp_event_base_t event_base, int32_t event_id, void *event_data)
{
    (void)arg;
    (void)event_data;

    if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START) {
        if (s_has_credentials) {
            esp_wifi_connect();
        } else {
            ESP_LOGI(TAG, "Wi-Fi started without credentials, waiting for provisioning");
        }
    } else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED) {
        ESP_LOGW(TAG, "Wi-Fi disconnected, retrying");
        s_connected = false;
        if (s_has_credentials) {
            esp_wifi_connect();
        }
        if (s_config.state_cb) {
            s_config.state_cb(NETWORK_STATE_DISCONNECTED, s_config.cb_ctx);
        }
    } else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP) {
        ESP_LOGI(TAG, "Wi-Fi connected");
        s_connected = true;
        if (s_config.state_cb) {
            s_config.state_cb(NETWORK_STATE_CONNECTED, s_config.cb_ctx);
        }
    }
}

esp_err_t network_manager_init(const network_manager_config_t *config)
{
    if (!config) {
        return ESP_ERR_INVALID_ARG;
    }

    s_config = *config;

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));

    ESP_ERROR_CHECK(esp_netif_create_default_wifi_sta());
    ESP_ERROR_CHECK(esp_netif_create_default_wifi_ap());

    ESP_ERROR_CHECK(esp_event_handler_register(WIFI_EVENT, ESP_EVENT_ANY_ID, &wifi_event_handler, NULL));
    ESP_ERROR_CHECK(esp_event_handler_register(IP_EVENT, IP_EVENT_STA_GOT_IP, &wifi_event_handler, NULL));

    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    ESP_ERROR_CHECK(esp_wifi_start());

    return ESP_OK;
}

esp_err_t network_manager_set_credentials(const char *ssid, const char *password)
{
    if (!ssid || !password) {
        return ESP_ERR_INVALID_ARG;
    }

    strlcpy(s_ssid, ssid, sizeof(s_ssid));
    strlcpy(s_password, password, sizeof(s_password));
    s_has_credentials = true;

    wifi_config_t wifi_config = {
        .sta = {
            .threshold.authmode = WIFI_AUTH_WPA2_PSK,
        },
    };
    strlcpy((char *)wifi_config.sta.ssid, s_ssid, sizeof(wifi_config.sta.ssid));
    strlcpy((char *)wifi_config.sta.password, s_password, sizeof(wifi_config.sta.password));

    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &wifi_config));
    return ESP_OK;
}

esp_err_t network_manager_start_sta(void)
{
    if (!s_has_credentials) {
        return ESP_ERR_INVALID_STATE;
    }
    ESP_LOGI(TAG, "Starting STA connection to %s", s_ssid);
    return esp_wifi_connect();
}

esp_err_t network_manager_start_ap(const char *ssid, const char *password)
{
    if (!ssid || !password) {
        return ESP_ERR_INVALID_ARG;
    }

    wifi_config_t ap_config = {
        .ap = {
            .ssid_len = 0,
            .channel = 1,
            .max_connection = 4,
            .authmode = WIFI_AUTH_WPA_WPA2_PSK,
        },
    };
    strlcpy((char *)ap_config.ap.ssid, ssid, sizeof(ap_config.ap.ssid));
    ap_config.ap.ssid_len = strlen(ssid);

    if (strlen(password) == 0) {
        ap_config.ap.authmode = WIFI_AUTH_OPEN;
    } else {
        strlcpy((char *)ap_config.ap.password, password, sizeof(ap_config.ap.password));
    }

    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_APSTA));
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_AP, &ap_config));
    s_ap_running = true;
    ESP_LOGI(TAG, "SoftAP started as %s", ssid);
    return ESP_OK;
}

esp_err_t network_manager_stop_ap(void)
{
    if (!s_ap_running) {
        return ESP_OK;
    }

    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    s_ap_running = false;
    ESP_LOGI(TAG, "SoftAP stopped");
    return ESP_OK;
}

bool network_manager_is_connected(void)
{
    return s_connected;
}

