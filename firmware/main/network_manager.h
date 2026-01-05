#pragma once

#include "esp_err.h"
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    NETWORK_STATE_DISCONNECTED = 0,
    NETWORK_STATE_CONNECTED,
} network_state_t;

typedef void (*network_state_cb_t)(network_state_t state, void *ctx);

typedef struct {
    const char *ssid;
    const char *password;
    network_state_cb_t state_cb;
    void *cb_ctx;
} network_manager_config_t;

esp_err_t network_manager_init(const network_manager_config_t *config);
bool network_manager_is_connected(void);

#ifdef __cplusplus
}
#endif

