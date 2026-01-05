idf_component_register(
    SRCS "lvgl_stub.c" "lvgl_port.c" "st7796_display.c" "touch_driver.c"
    INCLUDE_DIRS "include"
    REQUIRES esp_timer driver
)

