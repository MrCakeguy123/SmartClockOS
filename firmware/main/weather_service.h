#pragma once

#include "esp_err.h"
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct weather_data_t {
    double temp_f;       // Current temperature in Fahrenheit
    double feels_like_f; // Apparent temperature
    double high_f;       // Today's high
    double low_f;        // Today's low
    int weather_code;    // WMO weather code
    char condition[32];  // Weather description
    char sunrise[8];     // e.g., "7:15 AM"
    char sunset[8];      // e.g., "5:42 PM"
} weather_data_t;

typedef void (*weather_update_cb_t)(const weather_data_t *data, void *ctx);

typedef struct {
    weather_update_cb_t update_cb;
    void *cb_ctx;
} weather_service_config_t;

esp_err_t weather_service_init(const weather_service_config_t *config);
void weather_service_request_update(void);

#ifdef __cplusplus
}
#endif

