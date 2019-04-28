/*******************************************************************************

 * nodecode  ver 0.1

 *
 * mam -1/21/2019
 *
 *******************************************************************************/

#include <stddef.h>
#include <stdio.h>
#include <string.h>

#include "nodecode.h"
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

#include "../../../headers/HAScore.h"
#include "../../../headers/HAStypedefs.h"

/* FreeRTOS event group to signal when we are connected & ready to make a request */
static EventGroupHandle_t wifi_event_group;

/* The event group allows multiple bits for each event,
   but we only care about one event - are we connected
   to the AP with an IP? */
const int CONNECTED_BIT = BIT0;

#define MQTT_CLIENT_THREAD_NAME         "mqtt_client_thread"
#define MQTT_CLIENT_THREAD_STACK_WORDS  4096
#define MQTT_CLIENT_THREAD_PRIO         8

#define MQTT_PUBLISH_TOPIC              "258Thomas/temp/location"
#define MQTT_SUBSCRIBE_TOPIC            "258Thomas/temp/location"
#define SENSOR_READ_THREAD_NAME         "sensor_read"
#define SENSOR_READ_THREAD_STACK_WORDS  4096
#define SENSOR_READ_THREAD_PRIO         8
#define SENSOR_READ_DELAY               10000000

#define WIFI_SSID                       "FrontierHSI"
#define WIFI_PASSWORD                   ""
#define MQTT_BROKER                     "192.168.254.221"
#define MQTT_PORT                       1883

static const char *TAG = "nodecode";

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
            .ssid = WIFI_SSID,
            .password = WIFI_PASSWORD,
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

static void mqtt_client_thread(_SENSOR_DATA *pvParameters)
{
    char *payload = NULL;
    MQTTClient client;
    Network network;
    int rc = 0;
    char clientID[32] = {0};

    ESP_LOGI(TAG, "ssid:%s passwd:%s sub:%s qos:%u pub:%s qos:%u pubinterval:%u payloadsize:%u",
             WIFI_SSID, WIFI_PASSWORD, MQTT_SUBSCRIBE_TOPIC,
             CONFIG_DEFAULT_MQTT_SUB_QOS, CONFIG_MQTT_PUB_TOPIC, CONFIG_DEFAULT_MQTT_PUB_QOS,
             CONFIG_MQTT_PUBLISH_INTERVAL, CONFIG_MQTT_PAYLOAD_BUFFER);

    ESP_LOGI(TAG, "ver:%u clientID:%s keepalive:%d username:%s passwd:%s session:%d level:%u",
             CONFIG_DEFAULT_MQTT_VERSION, CONFIG_MQTT_CLIENT_ID,
             CONFIG_MQTT_KEEP_ALIVE, CONFIG_MQTT_USERNAME, CONFIG_MQTT_PASSWORD,
             CONFIG_DEFAULT_MQTT_SESSION, CONFIG_DEFAULT_MQTT_SECURITY);

    ESP_LOGI(TAG, "broker:%s port:%u", MQTT_BROKER, MQTT_PORT);

    ESP_LOGI(TAG, "sendbuf:%u recvbuf:%u sendcycle:%u recvcycle:%u",
             CONFIG_MQTT_SEND_BUFFER, CONFIG_MQTT_RECV_BUFFER,
             CONFIG_MQTT_SEND_CYCLE, CONFIG_MQTT_RECV_CYCLE);
    ESP_LOGI(TAG, "temp - %d, humid - %d", (*pvParameters).temperature,(*pvParameters).humidity);

    MQTTPacket_connectData                  connectData = MQTTPacket_connectData_initializer;

    NetworkInit(&network);

    if (MQTTClientInit(&client, &network, 0, NULL, 0, NULL, 0) == false)
    {
        ESP_LOGE(TAG, "mqtt init err");
        vTaskDelete(NULL);
    }

    payload = malloc(CONFIG_MQTT_PAYLOAD_BUFFER);

    if (!payload)
    {
        ESP_LOGE(TAG, "mqtt malloc err");
    }
    else
    {
        memset(payload, 0x0, CONFIG_MQTT_PAYLOAD_BUFFER);
    }

    ESP_LOGI(TAG, "wait wifi connect...");
    xEventGroupWaitBits(wifi_event_group, CONNECTED_BIT, false, true, portMAX_DELAY);

    if ((rc = NetworkConnect(&network, MQTT_BROKER, MQTT_PORT)) != 0)
    {
        ESP_LOGE(TAG, "Return code from network connect is %d", rc);

    }

    connectData.MQTTVersion = CONFIG_DEFAULT_MQTT_VERSION;

    sprintf(clientID, "%s_%u", CONFIG_MQTT_CLIENT_ID, esp_random());

    connectData.clientID.cstring = clientID;
    connectData.keepAliveInterval = CONFIG_MQTT_KEEP_ALIVE;

    connectData.username.cstring = CONFIG_MQTT_USERNAME;
    connectData.password.cstring = CONFIG_MQTT_PASSWORD;

    connectData.cleansession = CONFIG_DEFAULT_MQTT_SESSION;

    ESP_LOGI(TAG, "MQTT Connecting");

    if ((rc = MQTTConnect(&client, &connectData)) != 0)
    {
        ESP_LOGE(TAG, "Return code from MQTT connect is %d", rc);
        network.disconnect(&network);
    }


    ESP_LOGI(TAG, "MQTT Connected");

#if defined(MQTT_TASK)

    if ((rc = MQTTStartTask(&client)) != pdPASS)
    {
        ESP_LOGE(TAG, "Return code from start tasks is %d", rc);
    }
    else
    {
        ESP_LOGI(TAG, "Use MQTTStartTask");
    }

#endif


    MQTTMessage message;

    message.qos = CONFIG_DEFAULT_MQTT_PUB_QOS;
    message.retained = 0;
    message.payload = payload;
    sprintf(payload, "temperature %d", (*pvParameters).temperature);
            message.payloadlen = strlen(payload);

    if ((rc = MQTTPublish(&client, "258thomas/house/tempertature", &message)) != 0)
    {
        ESP_LOGE(TAG, "Return code from MQTT publish is %d", rc);
    }
    else
    {
        ESP_LOGI(TAG, "MQTT published topic %s, len:%u heap:%u", CONFIG_MQTT_PUB_TOPIC, message.payloadlen, esp_get_free_heap_size());
    }

    if (rc != 0)
    {
        // break;
    }
    memset(payload, 0x0, CONFIG_MQTT_PAYLOAD_BUFFER);
    sprintf(payload, "humidity %d", (*pvParameters).humidity);
    message.payloadlen = strlen(payload);

    if ((rc = MQTTPublish(&client, "258thomas/house/humidity", &message)) != 0)
    {
        ESP_LOGE(TAG, "Return code from MQTT publish is %d", rc);
    }
    else
    {
        ESP_LOGI(TAG, "MQTT published topic %s, len:%u heap:%u", CONFIG_MQTT_PUB_TOPIC, message.payloadlen, esp_get_free_heap_size());
    }

    if (rc != 0)
    {
        // break;
    }

    network.disconnect(&network);
    ESP_LOGW(TAG, "mqtt_client_thread going to be deleted");
    vTaskDelete(NULL);
    return;
}

static void read_sensor_cb(void *pvParameters)
{
    _SENSOR_DATA            sensor_data;


    // read sensor
    printf("%s\n", "read sensor");
    sensor_data.temperature = 88;
    sensor_data.humidity = 66;
    printf("%s\n", "publish reading");


    // initialise_wifi();
    // printf("%s\n", "wifi initialized");


    // Initialize NVS
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES)
    {
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

    if (ret != pdPASS)
    {
        ESP_LOGE(TAG, "mqtt create  %s failed", MQTT_CLIENT_THREAD_NAME);
    }


    // disconnect from wifi
    // ret = esp_wifi_disconnect();
    // switch(ret){
    //     case ESP_OK:
    //         printf("%s\n", "sucessful disconnect");
    //         break;
    //     case ESP_ERR_WIFI_NOT_INIT:
    //         printf("%s\n", "WiFi was not initialized by esp_wifi_init");
    //         break;
    //     case ESP_ERR_WIFI_NOT_STARTED:
    //         printf("%s\n", "WiFi was not started by esp_wifi_start");
    //         break;
    //     case ESP_FAIL:
    //         printf("%s\n", "other WiFi internal errors");
    //         break;
    // }

}


void app_main(void)
{
    // Initialize NVS
    // esp_err_t                           ret = nvs_flash_init();
    esp_err_t                           my_err_t;
    esp_timer_handle_t                  s_timer;
    uint64_t                            sensor_read_delay = SENSOR_READ_DELAY;


    printf("\nSDK version:%s\n", esp_get_idf_version());
    printf("%s\n", "HAS version 0.0");
    initialise_wifi();
    printf("%s\n", "wifi initialized");
    ESP_LOGI(TAG, "logit wifi intitialzed");
    printf("nodecode  version %s\n", _VERSION);

    // initialise_wifi();
    // printf("%s\n", "wifi initialized");
    // ESP_LOGI(TAG, "wifi intitialzed");


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
}
