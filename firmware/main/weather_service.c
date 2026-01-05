#include "weather_service.h"
#include "config.h"

#include "esp_log.h"
#include "esp_http_client.h"
#include "esp_crt_bundle.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>

static const char *TAG = "weather_service";

static weather_service_config_t s_config = {0};

// HTTP response buffer
#define HTTP_BUFFER_SIZE 4096
static char http_response_buffer[HTTP_BUFFER_SIZE];
static int http_response_len = 0;

// Weather code to description mapping (WMO codes)
static const char *get_weather_description(int code)
{
    switch (code) {
    case 0:
        return "Clear";
    case 1:
        return "Mostly Clear";
    case 2:
        return "Partly Cloudy";
    case 3:
        return "Overcast";
    case 45:
    case 48:
        return "Foggy";
    case 51:
    case 53:
    case 55:
        return "Drizzle";
    case 56:
    case 57:
        return "Freezing Drizzle";
    case 61:
    case 63:
    case 65:
        return "Rain";
    case 66:
    case 67:
        return "Freezing Rain";
    case 71:
    case 73:
    case 75:
        return "Snow";
    case 77:
        return "Snow Grains";
    case 80:
    case 81:
    case 82:
        return "Showers";
    case 85:
    case 86:
        return "Snow Showers";
    case 95:
        return "Thunderstorm";
    case 96:
    case 99:
        return "Thunderstorm w/ Hail";
    default:
        return "Unknown";
    }
}

// Helper function to parse a double value from JSON after a key
// Handles both direct values ("key":123) and arrays ("key":[123])
static double parse_json_double(const char *json, const char *key, size_t start_pos)
{
    char search[64];
    snprintf(search, sizeof(search), "\"%s\":", key);

    const char *pos = strstr(json + start_pos, search);
    if (pos != NULL) {
        pos += strlen(search);
        // Skip whitespace
        while (*pos == ' ' || *pos == '\t') pos++;
        // Check if it's an array
        if (*pos == '[') {
            pos++; // Skip '['
        }
        return atof(pos);
    }
    return 0.0;
}

// Helper function to parse an int value from JSON after a key
static int parse_json_int(const char *json, const char *key, size_t start_pos)
{
    char search[64];
    snprintf(search, sizeof(search), "\"%s\":", key);

    const char *pos = strstr(json + start_pos, search);
    if (pos != NULL) {
        pos += strlen(search);
        // Skip whitespace
        while (*pos == ' ' || *pos == '\t') pos++;
        // Check if it's an array
        if (*pos == '[') {
            pos++; // Skip '['
        }
        return atoi(pos);
    }
    return 0;
}

// Helper to parse time string from ISO format arrays
// sunrise/sunset come as arrays: "sunrise":["2026-01-04T07:56"],"sunset":["2026-01-04T17:42"]
static void parse_sun_time(const char *json, const char *key, char *out, size_t out_size)
{
    char search[64];
    snprintf(search, sizeof(search), "\"%s\":[\"", key);

    const char *pos = strstr(json, search);
    if (pos != NULL) {
        pos += strlen(search);
        // Find the 'T' separator and get time part
        const char *tpos = strchr(pos, 'T');
        if (tpos != NULL && (tpos - pos) < 20) {
            int hour = atoi(tpos + 1);
            int min = 0;
            const char *colon_pos = strchr(tpos, ':');
            if (colon_pos != NULL) {
                min = atoi(colon_pos + 1);
            }
            const char *ampm = (hour >= 12) ? "PM" : "AM";
            if (hour > 12) hour -= 12;
            if (hour == 0) hour = 12;
            snprintf(out, out_size, "%d:%02d %s", hour, min, ampm);
            return;
        }
    }
    snprintf(out, out_size, "--:--");
}

// HTTP event handler
static esp_err_t http_event_handler(esp_http_client_event_t *evt)
{
    switch (evt->event_id) {
    case HTTP_EVENT_ON_DATA:
        if (!esp_http_client_is_chunked_response(evt->client)) {
            if (http_response_len + evt->data_len < HTTP_BUFFER_SIZE - 1) {
                memcpy(http_response_buffer + http_response_len, evt->data, evt->data_len);
                http_response_len += evt->data_len;
                http_response_buffer[http_response_len] = '\0';
            }
        }
        break;
    default:
        break;
    }
    return ESP_OK;
}

// Fetch real weather from Open-Meteo API (no API key required)
static bool fetch_real_weather(weather_data_t *data)
{
    bool success = false;
    memset(data, 0, sizeof(weather_data_t));
    http_response_len = 0;
    memset(http_response_buffer, 0, sizeof(http_response_buffer));

    // Build URL
    char url[512];
    snprintf(url, sizeof(url),
             "https://api.open-meteo.com/v1/forecast?"
             "latitude=%.4f&longitude=%.4f"
             "&current=temperature_2m,apparent_temperature,weather_code"
             "&daily=temperature_2m_max,temperature_2m_min,sunrise,sunset"
             "&temperature_unit=fahrenheit&timezone=auto&forecast_days=1",
             WEATHER_LAT, WEATHER_LON);

    ESP_LOGI(TAG, "Fetching weather from: %s", url);

    esp_http_client_config_t config = {
        .url = url,
        .event_handler = http_event_handler,
        .crt_bundle_attach = esp_crt_bundle_attach,
        .timeout_ms = 10000,
    };

    esp_http_client_handle_t client = esp_http_client_init(&config);
    if (client == NULL) {
        ESP_LOGE(TAG, "Failed to initialize HTTP client");
        return false;
    }

    esp_err_t err = esp_http_client_perform(client);
    if (err == ESP_OK) {
        int status_code = esp_http_client_get_status_code(client);
        ESP_LOGI(TAG, "HTTP Status = %d, content_length = %d",
                 status_code, http_response_len);

        if (status_code == 200 && http_response_len > 0) {
            // Parse current section
            const char *current_pos = strstr(http_response_buffer, "\"current\":{");
            if (current_pos != NULL) {
                size_t offset = current_pos - http_response_buffer;

                data->temp_f = parse_json_double(http_response_buffer, "temperature_2m", offset);
                data->feels_like_f = parse_json_double(http_response_buffer, "apparent_temperature", offset);
                data->weather_code = parse_json_int(http_response_buffer, "weather_code", offset);

                const char *desc = get_weather_description(data->weather_code);
                snprintf(data->condition, sizeof(data->condition), "%s", desc);

                // Parse daily section for high/low and sun times
                const char *daily_pos = strstr(http_response_buffer, "\"daily\":{");
                if (daily_pos != NULL) {
                    size_t daily_offset = daily_pos - http_response_buffer;
                    data->high_f = parse_json_double(http_response_buffer, "temperature_2m_max", daily_offset);
                    data->low_f = parse_json_double(http_response_buffer, "temperature_2m_min", daily_offset);
                    parse_sun_time(http_response_buffer, "sunrise", data->sunrise, sizeof(data->sunrise));
                    parse_sun_time(http_response_buffer, "sunset", data->sunset, sizeof(data->sunset));
                }

                success = true;
                ESP_LOGI(TAG, "Fetched: %.0fF (feels %.0fF), Hi:%.0f Lo:%.0f, %s",
                         data->temp_f, data->feels_like_f, data->high_f, data->low_f,
                         data->condition);
                ESP_LOGI(TAG, "Sunrise: %s, Sunset: %s", data->sunrise, data->sunset);
            } else {
                ESP_LOGW(TAG, "Could not find 'current' section in response");
            }
        } else {
            ESP_LOGW(TAG, "HTTP request failed with status %d", status_code);
        }
    } else {
        ESP_LOGE(TAG, "HTTP request failed: %s", esp_err_to_name(err));
    }

    esp_http_client_cleanup(client);
    return success;
}

esp_err_t weather_service_init(const weather_service_config_t *config)
{
    if (!config || !config->update_cb) {
        return ESP_ERR_INVALID_ARG;
    }

    s_config = *config;
    ESP_LOGI(TAG, "Weather service initialized (using Open-Meteo API)");
    return ESP_OK;
}

void weather_service_request_update(void)
{
    ESP_LOGI(TAG, "Fetching weather data...");

    weather_data_t data;

    if (fetch_real_weather(&data)) {
        if (s_config.update_cb) {
            s_config.update_cb(&data, s_config.cb_ctx);
        }
    } else {
        ESP_LOGW(TAG, "Failed to fetch weather, sending offline data");
        // Send offline data
        memset(&data, 0, sizeof(data));
        snprintf(data.condition, sizeof(data.condition), "Offline");
        snprintf(data.sunrise, sizeof(data.sunrise), "--:--");
        snprintf(data.sunset, sizeof(data.sunset), "--:--");
        if (s_config.update_cb) {
            s_config.update_cb(&data, s_config.cb_ctx);
        }
    }
}
