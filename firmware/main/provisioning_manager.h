#pragma once

#include "esp_err.h"
#include "network_manager.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    network_state_cb_t state_cb;
    void *cb_ctx;
} provisioning_manager_config_t;

esp_err_t provisioning_manager_init(const provisioning_manager_config_t *config);

#ifdef __cplusplus
}
#endif
