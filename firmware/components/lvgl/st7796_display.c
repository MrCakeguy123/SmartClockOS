#include "st7796_display.h"

#include "esp_log.h"

static const char *TAG = "st7796";
static lv_obj_t s_root = {0};

esp_err_t st7796_display_init(void)
{
    ESP_LOGI(TAG, "Configuring ST7796 display (%dx%d RGB565)", LV_HOR_RES, LV_VER_RES);
    return ESP_OK;
}

lv_obj_t *st7796_get_root(void)
{
    return &s_root;
}

