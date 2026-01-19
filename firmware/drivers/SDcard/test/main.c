#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "SDcard.h"

#define CMD_NUM 10

void process(FILE *f, uint16_t commands_num){
    for(int i = 0; i < commands_num; i++){
        char line[MAX_CHAR_SIZE];
        fgets(line, sizeof(line), f);

        // Usuń znak nowej linii
        char *pos = strchr(line, '\n');
        if (pos) {
            *pos = '\0';
        }
        ESP_LOGI(SD_TAG, "Read from file: '%s'", line);
    }
}

void app_main(void)
{  
    // Initialization
    esp_err_t ret;
    sdmmc_card_t *card = NULL;
    ret = card_init(&card);
    if (ret != ESP_OK) {
        return;
    }

#ifdef CONFIG_FORMAT_SD_CARD
    ret = esp_vfs_fat_sdcard_format(mount_point, card);
    if (ret != ESP_OK) {
        ESP_LOGE(SD_TAG, "Failed to format FATFS (%s)", esp_err_to_name(ret));
        return;
    }

    if (stat(file_foo, &st) == 0) {
        ESP_LOGI(SD_TAG, "file still exists");
        return;
    } else {
        ESP_LOGI(SD_TAG, "file doesn't exist, formatting done");
    }
#endif

    // Writing to file
    char *code[][CMD_NUM] = 
    {
        {"G21"},
        {"G90"},
        {"G28 X0 Y0"},
        {"G0 Z10.0"},
        {"G0 X20.0 Y20.0"},
        {"G1 Z0.0 F1200"},
        {"G1 X60.0 Y60.0"},
        {"G1 X20.0 Y60.0"},
        {"G0 Z15.0"},
        {"M30"}
    };
    char data[MAX_CHAR_SIZE];
    const char *program = MOUNT_POINT"/program.gcode";
    FILE *f = fopen(program, "w"); fclose(f); // Clear file

    for(int i = 0; i < CMD_NUM; i++){
        memset(data, 0, MAX_CHAR_SIZE);
        snprintf(data, MAX_CHAR_SIZE, "%s\n", *code[i]); 
        ret = sd_write_file(program, data);
        if (ret != ESP_OK) {
                return;
        }
    }

    ret = sd_read_file(program, process);
    if (ret != ESP_OK) {
        return;
    }
}