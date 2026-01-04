#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_event.h"
#include "esp_log.h"
#include "esp_netif.h"
#include "esp_sntp.h"
#include "esp_system.h"
#include "esp_wifi.h"
#include "nvs_flash.h"
#include "lvgl.h"
#include <string.h>
#include <time.h>

static const char *TAG = "SmartClock";

// Forward declarations
static esp_err_t app_init_nvs(void);
static esp_err_t app_init_network(void);
static void app_start_sntp(void);
static void app_create_clock_ui(void);
static void app_lvgl_tick(void *arg);
static void app_fetch_weather(char *out, size_t len);

typedef struct {
    lv_obj_t *time_label;
    lv_obj_t *sub_label;
    lv_obj_t *weather_label;
    int weather_ticks;
} app_clock_ctx_t;

static void app_main_task(void *arg)
{
    ESP_LOGI(TAG, "booting SmartClockOS");

    ESP_ERROR_CHECK(app_init_nvs());
    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());
    ESP_ERROR_CHECK(app_init_network());

    app_start_sntp();

    lv_init();
    // Display and touch driver init should be placed here (platform-specific)

    // LVGL tick timer (assumes 1ms tick in separate task)
    xTaskCreate(app_lvgl_tick, "lv_tick", 2048, NULL, 5, NULL);

    app_create_clock_ui();

    while (true) {
        lv_task_handler();
        vTaskDelay(pdMS_TO_TICKS(5));
    }
}

static esp_err_t app_init_nvs(void)
{
    esp_err_t err = nvs_flash_init();
    if (err == ESP_ERR_NVS_NO_FREE_PAGES || err == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        err = nvs_flash_init();
    }
    return err;
}

static void wifi_event_handler(void *arg, esp_event_base_t event_base, int32_t event_id, void *event_data)
{
    if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START) {
        esp_wifi_connect();
    } else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED) {
        ESP_LOGW(TAG, "Wi-Fi disconnected, retrying");
        esp_wifi_connect();
    } else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP) {
        ESP_LOGI(TAG, "Wi-Fi connected");
    }
}

static esp_err_t app_init_network(void)
{
    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));

    ESP_ERROR_CHECK(esp_event_handler_register(WIFI_EVENT, ESP_EVENT_ANY_ID, &wifi_event_handler, NULL));
    ESP_ERROR_CHECK(esp_event_handler_register(IP_EVENT, IP_EVENT_STA_GOT_IP, &wifi_event_handler, NULL));

    wifi_config_t wifi_config = {
        .sta = {
            .ssid = "<your-ssid>",
            .password = "<your-password>",
            .threshold.authmode = WIFI_AUTH_WPA2_PSK,
        },
    };

    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &wifi_config));
    ESP_ERROR_CHECK(esp_wifi_start());

    return ESP_OK;
}

static void app_start_sntp(void)
{
    esp_sntp_config_t config = ESP_NETIF_SNTP_DEFAULT_CONFIG("pool.ntp.org");
    config.sync_cb = NULL;
    esp_netif_sntp_init(&config);
    esp_netif_sntp_start();
    ESP_LOGI(TAG, "SNTP started");
}

static void app_lvgl_tick(void *arg)
{
    (void)arg;
    const TickType_t delay = pdMS_TO_TICKS(1);
    while (true) {
        lv_tick_inc(1);
        vTaskDelay(delay);
    }
}

static void app_update_clock(lv_timer_t *timer)
{
    app_clock_ctx_t *ctx = (app_clock_ctx_t *)timer->user_data;

    time_t now = time(NULL);
    struct tm info = {0};
    localtime_r(&now, &info);

    char time_buf[8];
    strftime(time_buf, sizeof(time_buf), "%H:%M", &info);
    lv_label_set_text(ctx->time_label, time_buf);

    char sub_buf[32];
    strftime(sub_buf, sizeof(sub_buf), "%a", &info);
    strlcat(sub_buf, " • Paris", sizeof(sub_buf));
    lv_label_set_text(ctx->sub_label, sub_buf);

    ctx->weather_ticks++;
    if (ctx->weather_ticks >= 300) { // refresh roughly every 5 minutes
        char weather_buf[48];
        app_fetch_weather(weather_buf, sizeof(weather_buf));
        lv_label_set_text(ctx->weather_label, weather_buf);
        ctx->weather_ticks = 0;
    }
}

static void app_create_clock_ui(void)
{
    lv_obj_t *screen = lv_scr_act();

    // Background gradient
    static lv_style_t bg_style;
    lv_style_init(&bg_style);
    lv_style_set_bg_color(&bg_style, lv_color_hex(0x102030));
    lv_style_set_bg_grad_color(&bg_style, lv_color_hex(0x203040));
    lv_style_set_bg_grad_dir(&bg_style, LV_GRAD_DIR_VER);
    lv_obj_add_style(screen, &bg_style, 0);

    // Time label
    lv_obj_t *time_label = lv_label_create(screen);
    lv_obj_set_style_text_font(time_label, &lv_font_montserrat_48, 0);
    lv_obj_set_style_text_color(time_label, lv_color_white(), 0);
    lv_label_set_text(time_label, "00:00");
    lv_obj_center(time_label);

    // Seconds animation: subtle scale pulse
    lv_anim_t anim;
    lv_anim_init(&anim);
    lv_anim_set_var(&anim, time_label);
    lv_anim_set_values(&anim, 100, 103);
    lv_anim_set_exec_cb(&anim, (lv_anim_exec_xcb_t)lv_obj_set_style_transform_zoom);
    lv_anim_set_time(&anim, 1000);
    lv_anim_set_repeat_count(&anim, LV_ANIM_REPEAT_INFINITE);
    lv_anim_start(&anim);

    // Subtitle for date/location
    lv_obj_t *sub_label = lv_label_create(screen);
    lv_obj_set_style_text_font(sub_label, &lv_font_montserrat_18, 0);
    lv_label_set_text(sub_label, "-- • --");
    lv_obj_align(sub_label, LV_ALIGN_BOTTOM_MID, 0, -32);

    // Weather label
    lv_obj_t *weather_label = lv_label_create(screen);
    lv_obj_set_style_text_font(weather_label, &lv_font_montserrat_22, 0);
    lv_label_set_text(weather_label, "--°C • --");
    lv_obj_align(weather_label, LV_ALIGN_BOTTOM_MID, 0, -4);

    static app_clock_ctx_t ctx;
    ctx.time_label = time_label;
    ctx.sub_label = sub_label;
    ctx.weather_label = weather_label;
    ctx.weather_ticks = 300; // force immediate first refresh

    lv_timer_create(app_update_clock, 1000, &ctx);
}

static void app_fetch_weather(char *out, size_t len)
{
    // TODO: replace with HTTP client to a weather API using the device location.
    // Stubbed data keeps UI responsive while networking/weather integration is wired in.
    snprintf(out, len, "22°C • Clear Skies");
}

void app_main(void)
{
    xTaskCreate(app_main_task, "app_main_task", 4096 * 2, NULL, 5, NULL);
}
