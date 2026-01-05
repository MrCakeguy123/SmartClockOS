#include "provisioning_manager.h"

#include "esp_event.h"
#include "esp_http_server.h"
#include "esp_log.h"
#include "esp_netif.h"
#include "nvs.h"
#include "nvs_flash.h"
#include "cJSON.h"
#include <stdio.h>
#include <string.h>

#include "ui_shell.h"

#define WIFI_NAMESPACE "wifi"
#define WIFI_KEY_SSID "ssid"
#define WIFI_KEY_PASSWORD "password"

#define MAX_FAILURES 5
#define PORTAL_SSID "SmartClock-Setup"
#define PORTAL_PASSWORD "configureme"

static const char *TAG = "provisioning";

typedef struct {
    provisioning_manager_config_t config;
    int failure_count;
    bool portal_running;
    httpd_handle_t server;
} provisioning_ctx_t;

static provisioning_ctx_t s_ctx = {0};

static esp_err_t load_credentials(char *ssid, size_t ssid_len, char *password, size_t password_len, bool *has_creds)
{
    nvs_handle_t handle;
    esp_err_t err = nvs_open(WIFI_NAMESPACE, NVS_READONLY, &handle);
    if (err == ESP_ERR_NVS_NOT_FOUND) {
        *has_creds = false;
        return ESP_OK;
    } else if (err != ESP_OK) {
        return err;
    }

    size_t ssid_size = ssid_len;
    size_t pass_size = password_len;

    err = nvs_get_str(handle, WIFI_KEY_SSID, ssid, &ssid_size);
    if (err == ESP_ERR_NVS_NOT_FOUND) {
        *has_creds = false;
        nvs_close(handle);
        return ESP_OK;
    }

    if (err != ESP_OK) {
        nvs_close(handle);
        return err;
    }

    err = nvs_get_str(handle, WIFI_KEY_PASSWORD, password, &pass_size);
    nvs_close(handle);

    if (err == ESP_ERR_NVS_NOT_FOUND) {
        *has_creds = false;
        return ESP_OK;
    }

    if (err != ESP_OK) {
        return err;
    }

    *has_creds = true;
    return ESP_OK;
}

static esp_err_t save_credentials(const char *ssid, const char *password)
{
    nvs_handle_t handle;
    ESP_ERROR_CHECK(nvs_open(WIFI_NAMESPACE, NVS_READWRITE, &handle));
    ESP_ERROR_CHECK(nvs_set_str(handle, WIFI_KEY_SSID, ssid));
    ESP_ERROR_CHECK(nvs_set_str(handle, WIFI_KEY_PASSWORD, password));
    ESP_ERROR_CHECK(nvs_commit(handle));
    nvs_close(handle);
    return ESP_OK;
}

static void stop_portal(void)
{
    if (s_ctx.portal_running) {
        if (s_ctx.server) {
            httpd_stop(s_ctx.server);
            s_ctx.server = NULL;
        }
        network_manager_stop_ap();
        s_ctx.portal_running = false;
        ESP_LOGI(TAG, "Provisioning portal stopped");
    }
}

static esp_err_t root_get_handler(httpd_req_t *req)
{
    const char *resp = "{""name"":""SmartClockOS"",""status"":""waiting""}";
    httpd_resp_set_type(req, "application/json");
    return httpd_resp_send(req, resp, strlen(resp));
}

static esp_err_t provision_post_handler(httpd_req_t *req)
{
    char buf[256] = {0};
    int recv_len = httpd_req_recv(req, buf, sizeof(buf) - 1);
    if (recv_len <= 0) {
        return httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, "Failed to read body");
    }

    cJSON *json = cJSON_Parse(buf);
    if (!json) {
        return httpd_resp_send_err(req, HTTPD_400_BAD_REQUEST, "Invalid JSON");
    }

    cJSON *ssid = cJSON_GetObjectItem(json, "ssid");
    cJSON *password = cJSON_GetObjectItem(json, "password");
    if (!cJSON_IsString(ssid) || !cJSON_IsString(password)) {
        cJSON_Delete(json);
        return httpd_resp_send_err(req, HTTPD_400_BAD_REQUEST, "ssid/password required");
    }

    ESP_LOGI(TAG, "Received new credentials for %s", ssid->valuestring);
    save_credentials(ssid->valuestring, password->valuestring);
    network_manager_set_credentials(ssid->valuestring, password->valuestring);
    network_manager_start_sta();

    ui_shell_show_onboarding("Connecting...", ssid->valuestring);

    cJSON *resp = cJSON_CreateObject();
    cJSON_AddStringToObject(resp, "status", "connecting");
    char *resp_str = cJSON_PrintUnformatted(resp);

    httpd_resp_set_type(req, "application/json");
    httpd_resp_send(req, resp_str, strlen(resp_str));

    cJSON_free(resp_str);
    cJSON_Delete(resp);
    cJSON_Delete(json);
    return ESP_OK;
}

static httpd_handle_t start_http_server(void)
{
    httpd_config_t config = HTTPD_DEFAULT_CONFIG();
    config.server_port = 80;
    config.lru_purge_enable = true;

    httpd_handle_t server = NULL;
    if (httpd_start(&server, &config) != ESP_OK) {
        ESP_LOGE(TAG, "Failed to start HTTP server");
        return NULL;
    }

    httpd_uri_t root = {
        .uri = "/",
        .method = HTTP_GET,
        .handler = root_get_handler,
        .user_ctx = NULL,
    };

    httpd_uri_t provision = {
        .uri = "/api/provision",
        .method = HTTP_POST,
        .handler = provision_post_handler,
        .user_ctx = NULL,
    };

    httpd_register_uri_handler(server, &root);
    httpd_register_uri_handler(server, &provision);
    ESP_LOGI(TAG, "HTTP portal available on http://192.168.4.1/");
    return server;
}

static void start_portal(void)
{
    if (s_ctx.portal_running) {
        return;
    }

    network_manager_start_ap(PORTAL_SSID, PORTAL_PASSWORD);
    s_ctx.server = start_http_server();
    if (!s_ctx.server) {
        ESP_LOGE(TAG, "Failed to start provisioning HTTP server");
        return;
    }
    s_ctx.portal_running = true;
    ui_shell_show_onboarding("Connect to setup Wi-Fi", PORTAL_SSID);
    ESP_LOGI(TAG, "Provisioning portal started");
}

static void on_network_event(network_state_t state, void *ctx)
{
    provisioning_ctx_t *prov = (provisioning_ctx_t *)ctx;

    if (state == NETWORK_STATE_CONNECTED) {
        prov->failure_count = 0;
        ui_shell_show_onboarding("Connected", "Wi-Fi ready");
        stop_portal();
    } else {
        prov->failure_count++;
        char subtitle[32];
        snprintf(subtitle, sizeof(subtitle), "Retry %d/%d", prov->failure_count, MAX_FAILURES);
        ui_shell_show_onboarding("Connecting to Wi-Fi", subtitle);

        if (prov->failure_count >= MAX_FAILURES) {
            start_portal();
        }
    }

    if (prov->config.state_cb) {
        prov->config.state_cb(state, prov->config.cb_ctx);
    }
}

esp_err_t provisioning_manager_init(const provisioning_manager_config_t *config)
{
    if (!config) {
        return ESP_ERR_INVALID_ARG;
    }

    s_ctx.config = *config;
    s_ctx.failure_count = 0;
    s_ctx.portal_running = false;

    network_manager_config_t net_cfg = {
        .state_cb = on_network_event,
        .cb_ctx = &s_ctx,
    };
    ESP_ERROR_CHECK(network_manager_init(&net_cfg));

    char ssid[32] = {0};
    char password[64] = {0};
    bool has_creds = false;
    ESP_ERROR_CHECK(load_credentials(ssid, sizeof(ssid), password, sizeof(password), &has_creds));

    if (has_creds) {
        ESP_LOGI(TAG, "Found stored credentials for %s", ssid);
        network_manager_set_credentials(ssid, password);
        ui_shell_show_onboarding("Connecting to Wi-Fi", ssid);
        network_manager_start_sta();
    } else {
        ESP_LOGW(TAG, "No stored credentials, starting provisioning portal");
        start_portal();
    }

    return ESP_OK;
}
