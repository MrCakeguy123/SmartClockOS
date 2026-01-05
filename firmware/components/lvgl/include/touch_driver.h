#pragma once

#include "esp_err.h"
#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

esp_err_t touch_driver_init(void);
bool touch_driver_read(uint16_t *x, uint16_t *y, bool *touched);

#ifdef __cplusplus
}
#endif

