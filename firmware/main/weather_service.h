#pragma once

#include "esp_err.h"
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    char description[48];
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

