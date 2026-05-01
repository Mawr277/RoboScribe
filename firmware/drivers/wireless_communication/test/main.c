#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "nvs_flash.h"
#include "SDcard.h"
#include "wireless_communication.h"

void app_main(void)
{

    // Initialization of SD CARD
    esp_err_t ret;
    sdmmc_card_t *card = NULL;
    ret = card_init(&card);
    if (ret != ESP_OK) {
        ESP_LOGE("MAIN_APP", "SD Card failed to initialize! Stopping...");
        return;
    }
}
