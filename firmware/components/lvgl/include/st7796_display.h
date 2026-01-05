#pragma once

#include "esp_err.h"
#include "lvgl.h"

#ifdef __cplusplus
extern "C" {
#endif

esp_err_t st7796_display_init(void);
lv_obj_t *st7796_get_root(void);

#ifdef __cplusplus
}
#endif

