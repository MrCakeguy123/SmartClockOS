#include "touch_driver.h"

#include "esp_log.h"

static const char *TAG = "touch_driver";

esp_err_t touch_driver_init(void)
{
    ESP_LOGI(TAG, "Configuring capacitive touch controller via I2C");
    return ESP_OK;
}

bool touch_driver_read(uint16_t *x, uint16_t *y, bool *touched)
{
    if (x) {
        *x = 0;
    }
    if (y) {
        *y = 0;
    }
    if (touched) {
        *touched = false;
    }
    return false;
}

