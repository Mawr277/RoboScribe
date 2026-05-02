#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "nvs_flash.h"
#include "SDcard.h"
#include "wireless_communication.h"

// Osobny tag dla głównego pliku aplikacji
static const char *APP_TAG = "MAIN_APP";

void app_main(void)
{

    // 1. Inicjalizacja NVS (Pamięć wymagana do zapisania ustawień Wi-Fi)
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        nvs_flash_erase();
        nvs_flash_init();
    }

    // 2. Inicjalizacja Karty SD
    sdmmc_card_t *card = NULL;
    if (card_init(&card) != ESP_OK) {
        ESP_LOGE(APP_TAG, "SD Card failed to initialize! Stopping...");
        return; // Bez karty SD praca serwera nie ma sensu
    }

    // 3. Inicjalizacja sieci Wi-Fi
    wifi_init_sta();

    // 4. Uruchomienie Serwera HTTP
    if (start_webserver() != ESP_OK) {
        ESP_LOGE(APP_TAG, "Webserver failed to start! Stopping...");
        return; 
    }

    //ESP_LOGI(APP_TAG, "System ready! Check the ESP IP address above.");
    //ESP_LOGI(APP_TAG, "Send a file via POST to: http://<IP_ESP>/upload");

    // 5. Pętla główna programu
    while(1) {
        vTaskDelay(pdMS_TO_TICKS(5000));
    }
}
