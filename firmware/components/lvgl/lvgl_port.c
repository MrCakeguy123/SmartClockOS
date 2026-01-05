#include "lvgl_port.h"
#include "st7796_display.h"
#include "touch_driver.h"

#include "esp_log.h"
#include "esp_check.h"

static const char *TAG = "lvgl_port";

esp_err_t lvgl_port_init(void)
{
    ESP_LOGI(TAG, "Initializing LVGL core");
    lv_init();

    ESP_LOGI(TAG, "Initializing ST7796 display driver");
    ESP_RETURN_ON_ERROR(st7796_display_init(), TAG, "display init failed");

    ESP_LOGI(TAG, "Initializing touch driver");
    ESP_RETURN_ON_ERROR(touch_driver_init(), TAG, "touch init failed");

    return ESP_OK;
}

