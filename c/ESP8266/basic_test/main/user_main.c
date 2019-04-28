
#include <stdio.h>
#include <string.h>
#include <stddef.h>

#include "esp_system.h"
#include "esp_timer.h"
#include "FreeRTOS.h"
#include "freertos/queue.h"


#include "esp_wifi.h"
#include "esp_event_loop.h"
#include "esp_log.h"
#include "nvs_flash.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"


#define ESP_TIMER_PERIOD 100 * 1000

QueueHandle_t  						q=NULL;
esp_timer_handle_t 					timer;
esp_err_t							my_err_t;
uint64_t 							timeout_us = ESP_TIMER_PERIOD;
int 								* timeout;

static void test_timer_cb(void *p)
{

	printf("%s\n", "***********************\n   timer triggered\n");
	printf("timer timeout = %i\n", *(int *)p);
	*(int *)p = true;
	printf("timer timeout = %i\n", *(int *)p);
	printf("%s\n", "***********************");
}

static void wifi_thread(int *pvParameters) {

	while (1) {
		printf("wifi thread - timeout = %i\n", *pvParameters);
		vTaskDelay(110);
	}
}



void app_main(void)
{
	

	printf("\nSDK version:%s\n", esp_get_idf_version());
	printf("%s\n", "HAS version 0.0\n");
	printf("%s\n", "create a timer");

	timeout = (int *)malloc(sizeof(*timeout));

	// define timer arguments
    esp_timer_create_args_t timer_args = {
        .callback = test_timer_cb,
        .arg = timeout,
        .dispatch_method = ESP_TIMER_TASK,
        .name = "test_timer"
    };

    // creat timer
	my_err_t = esp_timer_create(&timer_args, &timer);
	switch(my_err_t){
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
	*timeout = false;
	printf("timeout = %i\n",	*timeout );
	my_err_t = esp_timer_start_once(timer, timeout_us);
	switch(my_err_t){
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
			printf("%s\n", "esp_timer_start_once returned an unknown return code");
			break;
	}

	// Initialize NVS
    esp_err_t ret = nvs_flash_init();

    if (ret == ESP_ERR_NVS_NO_FREE_PAGES) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }

    ESP_ERROR_CHECK(ret);

    // initialise_wifi();

    ret = xTaskCreate(wifi_thread,
                      "wifi_thread",
                      500,
                      timeout,
                      4 ,
                      NULL);
    
}


// #include <assert.h>
// #include <stdint.h>
// #include <stdio.h>
// #include <string.h>
// #include <sys/param.h>

// // #include <unity.h>
// #include <esp_spi_flash.h>
// #include "esp_attr.h"
// #include "esp_log.h"
// #include "esp_timer.h"

// #include "FreeRTOS.h"
// #include "queue.h"
// #include "semphr.h"

// #define ESP_NUM_MAX 5
// #define ESP_TIMER_PERIOD 50 * 1000

// static void test_timer_cb(void *p)
// {
//     SemaphoreHandle_t sem = (SemaphoreHandle_t)p;

//     xSemaphoreGive(sem);
// }

// TEST_CASE("Test esp_timer create and delete once mode", "[log]")
// {
//     SemaphoreHandle_t sem;
//     esp_timer_handle_t timer[ESP_NUM_MAX];

//     sem = xSemaphoreCreateCounting(ESP_NUM_MAX, 0);
//     TEST_ASSERT_NOT_NULL(sem);

//     esp_timer_create_args_t timer_args = {
//         .callback = test_timer_cb,
//         .arg = NULL,
//         .dispatch_method = ESP_TIMER_TASK,
//         .name = "test_timer",
//     };

//     for (int i = 0; i < ESP_NUM_MAX; i++) {
//         timer[i] = NULL;
//     }

//     for (int i = 0; i < ESP_NUM_MAX; i++) {
//         TEST_ESP_OK(esp_timer_create(&timer_args, &timer[i]));
//         TEST_ESP_OK(esp_timer_start_once(timer[i], ESP_TIMER_PERIOD));
//     }

//     for (int i = 0; i < ESP_NUM_MAX; i++) {
//         TEST_ASSERT_EQUAL_HEX32(pdPASS, xSemaphoreTake(sem, portMAX_DELAY));
//     }

//     TEST_ASSERT_EQUAL_HEX32(pdFALSE, xSemaphoreTake(sem, 1000 / portMAX_DELAY));

//     for (int i = 0; i < ESP_NUM_MAX; i++) {
//         TEST_ESP_OK(esp_timer_delete(timer[i]));
//         timer[i] = NULL;
//     }

//     vSemaphoreDelete(sem);
// }

// TEST_CASE("Test esp_timer create and delete once mode", "[log]")
// {
//     SemaphoreHandle_t sem;
//     esp_timer_handle_t timer[ESP_NUM_MAX];

//     sem = xSemaphoreCreateCounting(ESP_NUM_MAX, 0);
//     TEST_ASSERT_NOT_NULL(sem);

//     esp_timer_create_args_t timer_args = {
//         .callback = test_timer_cb,
//         .arg = sem,
//         .dispatch_method = ESP_TIMER_TASK,
//         .name = "test_timer",
//     };

//     for (int i = 0; i < ESP_NUM_MAX; i++) {
//         timer[i] = NULL;
//     }

//     for (int i = 0; i < ESP_NUM_MAX; i++) {
//         TEST_ESP_OK(esp_timer_create(&timer_args, &timer[i]));
//         TEST_ESP_OK(esp_timer_start_periodic(timer[i], ESP_TIMER_PERIOD));
//     }

//     for (int i = 0; i < ESP_NUM_MAX * 4; i++) {
//         TEST_ASSERT_EQUAL_HEX32(pdPASS, xSemaphoreTake(sem, portMAX_DELAY));
//     }

//     for (int i = 0; i < ESP_NUM_MAX; i++) {
//         TEST_ESP_OK(esp_timer_delete(timer[i]));
//         timer[i] = NULL;
//     }

//     vSemaphoreDelete(sem);
// }

// // #include <stdio.h>

// // #include "esp_system.h"
// // #include "esp_timer.h"

// // #define ESP_NUM_MAX 5
// // #define ESP_TIMER_PERIOD 50 * 1000


// // esp_timer_handle_t 					my_timer[ESP_NUM_MAX];
// // esp_err_t							my_err_t;
// // static esp_timer_create_args_t		my_args;
// // uint64_t 							timeout_us = 100;


// // static void called(void *p)
// // {
// // 	printf("%s\n", "***********************\ntimer triggered\n*************************L");
// // }

// // void app_main(void)
// // {
// // 	printf("SDK version:%s\n", esp_get_idf_version());
// // 	printf("%s\n", "HAS version 0.0\n");
// // 	printf("%s\n", "create a timer");
// // 	my_args.callback = (called)caller;
// // 	my_args.arg = NULL;
// // 	my_args.dispatch_method = ESP_TIMER_TASK;
// // 	my_args.name = "t0001";

// //  	esp_timer_create_args_t timer_args = {
// //         .callback = called,
// //         .arg = NULL,
// //         .dispatch_method = ESP_TIMER_TASK,
// //         .name = "test_timer",





// // 	my_err_t = esp_timer_create(&my_args, my_timer);
// // 	switch(my_err_t){
// // 		case ESP_OK:
// // 			printf("%s\n", "timer created");
// // 			break;
// // 		case ESP_ERR_INVALID_ARG:
// // 			printf("%s\n", "the handle is invalid"); 
// // 			break;
// // 		case ESP_ERR_INVALID_STATE:
// // 			printf("%s\n", "the timer is already running");
// // 			break;
// // 		default :
// // 			printf("%s\n", "esp_timer_create returned an unknown return code");
// // 			break;
// // 	}
// // 	printf("%s\n", "start timer");
// // 	my_err_t = esp_timer_start_once(&my_timer, timeout_us);
// // 	switch(my_err_t){
// // 		case ESP_OK:
// // 			printf("%s\n", "success");
// // 			break;
// // 		case ESP_ERR_INVALID_ARG:
// // 			printf("%s\n", "the handle is invalid");
// // 			break;
// // 		case ESP_ERR_INVALID_STATE:
// // 			printf("%s\n", "the timer timer is already running");
// // 			break;
// // 		default :
// // 			printf("%s\n", "esp_timer_start_once returned an unknown return code");
// // 			break;
// // 	}
// // }
