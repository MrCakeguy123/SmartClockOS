idf_component_register(
    SRCS "main.c" "network_manager.c" "time_service.c" "weather_service.c" "ui_shell.c" "provisioning_manager.c" "power_manager.c"
    INCLUDE_DIRS "."
    REQUIRES esp_wifi esp_event esp_netif esp_http_server esp_http_client nvs_flash json
)
