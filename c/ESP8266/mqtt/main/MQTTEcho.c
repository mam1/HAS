/*******************************************************************************
 * Copyright (c) 2014 IBM Corp.
 *
 * All rights reserved. This program and the accompanying materials
 * are made available under the terms of the Eclipse Public License v1.0
 * and Eclipse Distribution License v1.0 which accompany this distribution.
 *
 * The Eclipse Public License is available at
 *    http://www.eclipse.org/legal/epl-v10.html
 * and the Eclipse Distribution License is available at
 *   http://www.eclipse.org/org/documents/edl-v10.php.
 *
 * Contributors:
 *    Ian Craggs - initial API and implementation and/or initial documentation
 *******************************************************************************/

#include <stddef.h>
#include <stdio.h>
#include <string.h>

#include "esp_system.h"
#include "esp_wifi.h"
#include "esp_event_loop.h"
#include "esp_log.h"
#include "nvs_flash.h"
#include "esp_timer.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"

#include "MQTTClient.h"

/* FreeRTOS event group to signal when we are connected & ready to make a request */
static EventGroupHandle_t wifi_event_group;

/* The event group allows multiple bits for each event,
   but we only care about one event - are we connected
   to the AP with an IP? */
const int CONNECTED_BIT = BIT0;

#define MQTT_CLIENT_THREAD_NAME         "mqtt_client_thread"
#define MQTT_CLIENT_THREAD_STACK_WORDS  4096
#define MQTT_CLIENT_THREAD_PRIO         8 

#define SENSOR_READ_THREAD_NAME         "sensor_read"
#define SENSOR_READ_THREAD_STACK_WORDS  4096
#define SENSOR_READ_THREAD_PRIO         8 
#define SENSOR_READ_DELAY               10000000

static const char *TAG = "example";

static esp_err_t event_handler(void *ctx, system_event_t *event)
{
    switch (event->event_id) {
    case SYSTEM_EVENT_STA_START:
        esp_wifi_connect();
        break;

    case SYSTEM_EVENT_STA_GOT_IP:
        xEventGroupSetBits(wifi_event_group, CONNECTED_BIT);
        break;

    case SYSTEM_EVENT_STA_DISCONNECTED:
        /* This is a workaround as ESP32 WiFi libs don't currently
           auto-reassociate. */
        esp_wifi_connect();
        xEventGroupClearBits(wifi_event_group, CONNECTED_BIT);
        break;

    default:
        break;
    }

    return ESP_OK;
}

static void initialise_wifi(void)
{
    tcpip_adapter_init();
    wifi_event_group = xEventGroupCreate();
    ESP_ERROR_CHECK(esp_event_loop_init(event_handler, NULL));
    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));
    ESP_ERROR_CHECK(esp_wifi_set_storage(WIFI_STORAGE_RAM));
    wifi_config_t wifi_config = {
        .sta = {
            .ssid = CONFIG_WIFI_SSID,
            .password = CONFIG_WIFI_PASSWORD,
        },
    };
    ESP_LOGI(TAG, "Setting WiFi configuration SSID %s...", wifi_config.sta.ssid);
    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    ESP_ERROR_CHECK(esp_wifi_set_config(ESP_IF_WIFI_STA, &wifi_config));
    ESP_ERROR_CHECK(esp_wifi_start());
}

static void messageArrived(MessageData *data)
{
    ESP_LOGI(TAG, "Message arrived[len:%u]: %.*s", \
             data->message->payloadlen, data->message->payloadlen, (char *)data->message->payload);
}

static void mqtt_client_thread(void *pvParameters)
{
    char *payload = NULL;
    MQTTClient client;
    Network network;
    int rc = 0;
    char clientID[32] = {0};
    uint32_t count = 0;

    ESP_LOGI(TAG, "ssid:%s passwd:%s sub:%s qos:%u pub:%s qos:%u pubinterval:%u payloadsize:%u",
             CONFIG_WIFI_SSID, CONFIG_WIFI_PASSWORD, CONFIG_MQTT_SUB_TOPIC,
             CONFIG_DEFAULT_MQTT_SUB_QOS, CONFIG_MQTT_PUB_TOPIC, CONFIG_DEFAULT_MQTT_PUB_QOS,
             CONFIG_MQTT_PUBLISH_INTERVAL, CONFIG_MQTT_PAYLOAD_BUFFER);

    ESP_LOGI(TAG, "ver:%u clientID:%s keepalive:%d username:%s passwd:%s session:%d level:%u",
             CONFIG_DEFAULT_MQTT_VERSION, CONFIG_MQTT_CLIENT_ID,
             CONFIG_MQTT_KEEP_ALIVE, CONFIG_MQTT_USERNAME, CONFIG_MQTT_PASSWORD,
             CONFIG_DEFAULT_MQTT_SESSION, CONFIG_DEFAULT_MQTT_SECURITY);

    ESP_LOGI(TAG, "broker:%s port:%u", CONFIG_MQTT_BROKER, CONFIG_MQTT_PORT);

    ESP_LOGI(TAG, "sendbuf:%u recvbuf:%u sendcycle:%u recvcycle:%u",
             CONFIG_MQTT_SEND_BUFFER, CONFIG_MQTT_RECV_BUFFER,
             CONFIG_MQTT_SEND_CYCLE, CONFIG_MQTT_RECV_CYCLE);

    MQTTPacket_connectData                  connectData = MQTTPacket_connectData_initializer;

    NetworkInit(&network);

    if (MQTTClientInit(&client, &network, 0, NULL, 0, NULL, 0) == false) {
        ESP_LOGE(TAG, "mqtt init err");
        vTaskDelete(NULL);
    }

    payload = malloc(CONFIG_MQTT_PAYLOAD_BUFFER);

    if (!payload) {
        ESP_LOGE(TAG, "mqtt malloc err");
    } else {
        memset(payload, 0x0, CONFIG_MQTT_PAYLOAD_BUFFER);
    }

    for (;;) {
        ESP_LOGI(TAG, "wait wifi connect...");
        xEventGroupWaitBits(wifi_event_group, CONNECTED_BIT, false, true, portMAX_DELAY);

        if ((rc = NetworkConnect(&network, CONFIG_MQTT_BROKER, CONFIG_MQTT_PORT)) != 0) {
            ESP_LOGE(TAG, "Return code from network connect is %d", rc);
            continue;
        }

        connectData.MQTTVersion = CONFIG_DEFAULT_MQTT_VERSION;

        sprintf(clientID, "%s_%u", CONFIG_MQTT_CLIENT_ID, esp_random());

        connectData.clientID.cstring = clientID;
        connectData.keepAliveInterval = CONFIG_MQTT_KEEP_ALIVE;

        connectData.username.cstring = CONFIG_MQTT_USERNAME;
        connectData.password.cstring = CONFIG_MQTT_PASSWORD;

        connectData.cleansession = CONFIG_DEFAULT_MQTT_SESSION;

        ESP_LOGI(TAG, "MQTT Connecting");

        if ((rc = MQTTConnect(&client, &connectData)) != 0) {
            ESP_LOGE(TAG, "Return code from MQTT connect is %d", rc);
            network.disconnect(&network);
            continue;
        }

        ESP_LOGI(TAG, "MQTT Connected");

 #if defined(MQTT_TASK)

        if ((rc = MQTTStartTask(&client)) != pdPASS) {
            ESP_LOGE(TAG, "Return code from start tasks is %d", rc);
        } else {
            ESP_LOGI(TAG, "Use MQTTStartTask");
        }

 #endif

        if ((rc = MQTTSubscribe(&client, CONFIG_MQTT_SUB_TOPIC, CONFIG_DEFAULT_MQTT_SUB_QOS, messageArrived)) != 0) {
            ESP_LOGE(TAG, "Return code from MQTT subscribe is %d", rc);
            network.disconnect(&network);
            continue;
        }

        ESP_LOGI(TAG, "MQTT subscribe to topic %s OK", CONFIG_MQTT_SUB_TOPIC);

        for (;;) {
            MQTTMessage message;

            message.qos = CONFIG_DEFAULT_MQTT_PUB_QOS;
            message.retained = 0;
            message.payload = payload;
            sprintf(payload, "message number %d", ++count);
            message.payloadlen = strlen(payload);

            if ((rc = MQTTPublish(&client, CONFIG_MQTT_PUB_TOPIC, &message)) != 0) {
                ESP_LOGE(TAG, "Return code from MQTT publish is %d", rc);
            } else {
                ESP_LOGI(TAG, "MQTT published topic %s, len:%u heap:%u", CONFIG_MQTT_PUB_TOPIC, message.payloadlen, esp_get_free_heap_size());
            }

            if (rc != 0) {
                break;
            }

            vTaskDelay(CONFIG_MQTT_PUBLISH_INTERVAL / portTICK_RATE_MS);
        }

        network.disconnect(&network);
    }

    ESP_LOGW(TAG, "mqtt_client_thread going to be deleted");
    vTaskDelete(NULL);
    return;
}

// void sensor_publish(void *p) {
//     int                 tempertature = 88;
//     int                 humidity = 55;
//     esp_err_t           ret;

//     printf("%s\n", "setup for MQTT publish");
//     initialise_wifi();
//     printf("%s\n", "wifi initialized");

//     ret = xTaskCreate(&sensor_publish,
//                       "sensor_publish",
//                       MQTT_CLIENT_THREAD_STACK_WORDS,
//                       NULL,
//                       MQTT_CLIENT_THREAD_PRIO,
//                       NULL);

//     if (ret != pdPASS)  {
//         ESP_LOGE(TAG, "sensor_publish create client thread %s failed", MQTT_CLIENT_THREAD_NAME);
//     }

//     esp_wifi_deinit();
//     printf("%s\n",  "wifi deinitalized" );
//     printf("%s\n","sensor_publish thread going to kill itself");
//     esp_err_t esp_wifi_deinit(void);
//     vTaskDelete(NULL);
//     return;
// }

static void read_sensor_cb(void *pvParameters) {

    struct {
        int             tempertature;
        int             humidity;
    } sensor_data;


// read sensor
    printf("%s\n", "read sensor");
    sensor_data.tempertature = 88;
    sensor_data.humidity = 66;
    printf("%s\n", "publish reading");

    // initialise_wifi();
    // printf("%s\n", "wifi initialized");

// Initialize NVS
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }

    ESP_ERROR_CHECK(ret);

    ret = xTaskCreate(&mqtt_client_thread,
                      "sensor_publish",
                      MQTT_CLIENT_THREAD_STACK_WORDS,
                      &sensor_data,
                      MQTT_CLIENT_THREAD_PRIO,
                      NULL);

    if (ret != pdPASS)  {
        ESP_LOGE(TAG, "mqtt create  %s failed", MQTT_CLIENT_THREAD_NAME);
    }

}


void app_main(void)
{
    // Initialize NVS
    esp_err_t                           ret = nvs_flash_init();
    esp_err_t                           my_err_t;
    esp_timer_handle_t                  s_timer;
    uint64_t                            sensor_read_delay = SENSOR_READ_DELAY;

    initialise_wifi();
    printf("%s\n", "wifi initialized");

    // define timer arguments
    esp_timer_create_args_t timer_args = {
        .callback = read_sensor_cb,
        .arg = NULL,
        .dispatch_method = ESP_TIMER_TASK,
        .name = "read_sensor"
    };

    // create timer to perodically read and publish sensor data
    my_err_t = esp_timer_create(&timer_args, &s_timer);
    switch (my_err_t) {
    case ESP_OK:
        printf("%s\n", "timer created");
        break;
    case ESP_ERR_INVALID_ARG:
        printf("%s\n", "the handle is invalid");
        break;
    case ESP_ERR_INVALID_STATE:
        printf("%s\n", "the timer is already running");
        break;
    default :
        printf("%s\n", "esp_timer_create returned an unknown return code");
        break;
    }

    // start timer
    printf("%s\n", "start timer");
    my_err_t = esp_timer_start_periodic(s_timer, sensor_read_delay);
    switch (my_err_t) {
    case ESP_OK:
        printf("%s\n", "timer running");
        break;
    case ESP_ERR_INVALID_ARG:
        printf("%s\n", "the handle is invalid");
        break;
    case ESP_ERR_INVALID_STATE:
        printf("%s\n", "the timer timer is already running");
        break;
    default :
        printf("%s\n", "esp_timer_start_periodic returned an unknown return code");
    }

    // // Initialize NVS
    // if (ret == ESP_ERR_NVS_NO_FREE_PAGES) {
    //     ESP_ERROR_CHECK(nvs_flash_erase());
    //     ret = nvs_flash_init();
    // }

    // ESP_ERROR_CHECK(ret);

    // initialise_wifi();

    // ret = xTaskCreate(&mqtt_client_thread,
    //                   MQTT_CLIENT_THREAD_NAME,
    //                   MQTT_CLIENT_THREAD_STACK_WORDS,
    //                   NULL,
    //                   MQTT_CLIENT_THREAD_PRIO,
    //                   NULL);

    // if (ret != pdPASS)  {
    //     ESP_LOGE(TAG, "mqtt create client thread %s failed", MQTT_CLIENT_THREAD_NAME);
    // }


}
