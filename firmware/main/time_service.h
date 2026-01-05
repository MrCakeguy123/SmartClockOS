#pragma once

#include "esp_err.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef void (*time_sync_cb_t)(void *ctx);

typedef struct {
    const char *server;
    time_sync_cb_t sync_cb;
    void *cb_ctx;
} time_service_config_t;

esp_err_t time_service_init(const time_service_config_t *config);
esp_err_t time_service_start(void);

#ifdef __cplusplus
}
#endif

