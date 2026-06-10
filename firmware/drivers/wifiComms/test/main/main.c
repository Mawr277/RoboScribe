#include <stdlib.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include "esp_event.h"
#include "esp_log.h"
#include "esp_netif.h"
#include "esp_err.h"
#include "nvs_flash.h"

#include "file_server.h"
#include "protocol_examples_common.h"
#include "SDcard.h"
#include "robot_data.h"

#define CORE0   0
#define CORE1   1

static const char *TAG = "wifiComms";

robot_state current_state = STOP;
char filename[64];
accel_vector imu_readings = {0};
QueueHandle_t manual_mov_queue = NULL;
SemaphoreHandle_t imu_readings_mutex = NULL; 

static void create_server_task(void *pvParameters){
    while(1){
        /* This helper function configures Wi-Fi or Ethernet, as selected in menuconfig.
        * Read "Establishing Wi-Fi or Ethernet Connection" section in
        * examples/protocols/README.md for more information about this function.
        */
        if(example_connect() == ESP_OK){
            ESP_LOGI(TAG, "Network connected! Starting file server.");
            ESP_ERROR_CHECK(start_file_server(MOUNT_POINT));
            ESP_LOGI(TAG, "File server started");
            break;
        } else{
            ESP_LOGW(TAG, "Connection failed. Retrying in 10 seconds...");
            vTaskDelay(pdMS_TO_TICKS(10000));
        }
    }
    vTaskDelete(NULL);
}

static void read_accel_data(void *pvParameters){
    while (1)
    {   
        if(xSemaphoreTake(imu_readings_mutex, pdMS_TO_TICKS(500)) == pdTRUE){
            imu_readings.x_accel = rand()%6 + 1;
            imu_readings.y_accel = rand()%6 + 1;
            imu_readings.z_accel = rand()%6 + 1;
            // ESP_LOGI(TAG, "x_accel=%hu, y_accel=%hu, z_accel=%hu",
            //     imu_readings.x_accel,
            //     imu_readings.y_accel,
            //     imu_readings.z_accel
            // );
            xSemaphoreGive(imu_readings_mutex);
        }
        vTaskDelay(pdMS_TO_TICKS(2000));
    }
}

void app_main(void)
{
    ESP_LOGI(TAG, "Starting example");
    ESP_ERROR_CHECK(nvs_flash_init());
    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());

    /* Initialize file storage */
    sdmmc_card_t *card = NULL;
    ESP_ERROR_CHECK(card_init(&card));

    imu_readings_mutex = xSemaphoreCreateMutex();

    /* Start the file server */
    xTaskCreatePinnedToCore(create_server_task, "start_server", 4096, NULL, 5, NULL, CORE0);
    xTaskCreatePinnedToCore(read_accel_data, "read_imu_data", 4096, NULL, 6, NULL, CORE1);
}