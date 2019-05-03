
#include "driver/gpio.h"

#include <stdio.h>
#include <string.h>
#include <stddef.h>

#include "esp_system.h"
#include "esp_timer.h"
#include "FreeRTOS.h"
#include "freertos/queue.h"

#include "esp_wifi.h"

#include "esp_wifi.h"
#include "esp_event_loop.h"
#include "esp_log.h"
#include "nvs_flash.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"

const int WIFI_CONNECTED_BIT = BIT0;

esp_err_t event_handler(void *ctx, system_event_t *event)
{
    printf("%s\n", "* * * * * * * * * * * * * * * * * * *" );
    printf("%s  %i\n","event", event->event_id);
    switch(event->event_id) {
    case SYSTEM_EVENT_STA_START:
        esp_wifi_connect();
        break;
    case SYSTEM_EVENT_STA_GOT_IP:
        ESP_LOGI("vswifi", "got ip:%s",
                ip4addr_ntoa(&event->event_info.got_ip.ip_info.ip));
                // xEventGroupSetBits(wifi_event_group, WIFI_CONNECTED_BIT);
        break;
    case SYSTEM_EVENT_AP_STACONNECTED:
        ESP_LOGI("vswifi", "station:"MACSTR" join, AID=%d",
                 MAC2STR(event->event_info.sta_connected.mac),
                 event->event_info.sta_connected.aid);
        break;
    case SYSTEM_EVENT_AP_STADISCONNECTED:
        ESP_LOGI("vswifi", "station:"MACSTR"leave, AID=%d",
                 MAC2STR(event->event_info.sta_disconnected.mac),
                 event->event_info.sta_disconnected.aid);
        break;
    case SYSTEM_EVENT_STA_DISCONNECTED:
        esp_wifi_connect();
        // xEventGroupClearBits(wifi_event_group, WIFI_CONNECTED_BIT);
        break;
    default:
        printf("%s\n","??? no hit" );
        break;
    }
    return ESP_OK;
}

void app_main(void)
{
printf("SYSTEM_EVENT_STA_START:%i\n", SYSTEM_EVENT_STA_START);

    nvs_flash_init();
    tcpip_adapter_init();
    ESP_ERROR_CHECK( esp_event_loop_init(event_handler, NULL) );
    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK( esp_wifi_init(&cfg) );
    ESP_ERROR_CHECK( esp_wifi_set_storage(WIFI_STORAGE_RAM) );
    ESP_ERROR_CHECK( esp_wifi_set_mode(WIFI_MODE_STA) );
    wifi_config_t sta_config = {
        .sta = {
            .ssid = "FrontierHSI",
            .password = "",
            .bssid_set = "18:fe:34:f3:36:fb"
        }
    };
    ESP_ERROR_CHECK( esp_wifi_set_config(WIFI_IF_STA, &sta_config) );
    ESP_ERROR_CHECK( esp_wifi_start() );
    ESP_ERROR_CHECK( esp_wifi_connect() );

    printf("%s\n", "% % % % % %");

    // gpio_set_direction(GPIO_NUM_4, GPIO_MODE_OUTPUT);
    int level = 0;
    while (true) {
        printf("%s\n", "set gpip level");
        // gpio_set_level(GPIO_NUM_4, level);
        // level = !level;
        vTaskDelay(300 / portTICK_PERIOD_MS);
    }
}

