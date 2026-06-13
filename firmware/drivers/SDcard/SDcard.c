/**
 * @file SDcard.c
 * @author Maciej Nowicki (maciejnowicki1234@gmail.com)
 * @brief Program do obsługi zapisywania i czytania 
 *      z karty SD stosując interfejs SPI.
 * 
 * Szerszy opis zastosowanych funkcji znajduje się w dokumentacji: \n
 * <a href="https://docs.espressif.com/projects/esp-idf/en/stable/esp32/api-reference/storage/sdmmc.html">
 *       - SD/SDIO/MMC Driver
 * </a> \n
 * <a href="https://docs.espressif.com/projects/esp-idf/en/stable/esp32/api-reference/storage/fatfs.html#_CPPv423esp_vfs_fat_sdspi_mountPKcPK12sdmmc_host_tPK21sdspi_device_config_tPK26esp_vfs_fat_mount_config_tPP12sdmmc_card_t">
 *       - FAT Filesystem Support
 * </a> 
 * @version 1.0
 * @date 2026-01-13
 * @ingroup drivers
 * 
 * @copyright Copyright (c) 2026
 * 
 */
#include "SDcard.h"

static const char *SD_TAG = "SDcard";

esp_err_t card_init(sdmmc_card_t **card){
    esp_err_t ret;

    // Konfiguracja montażu systemu plików
    esp_vfs_fat_sdmmc_mount_config_t mount_config = {

// Formatowanie karty SD po nieudanym montowaniu
#ifdef CONFIG_FORMAT_IF_MOUNT_FAILED
        .format_if_mount_failed = true,
#else
        .format_if_mount_failed = false,
#endif
        .max_files = 5,
        .allocation_unit_size = 16 * 1024
    };
    const char mount_point[] = MOUNT_POINT;
    ESP_LOGI(SD_TAG, "Initializing SD card");
    // Uchwyt do hosta SDSPI
    sdmmc_host_t host = SDSPI_HOST_DEFAULT();

    //Konfiguracja szyny SPI
    spi_bus_config_t bus_cfg = {
        .mosi_io_num = PIN_NUM_MOSI,
        .miso_io_num = PIN_NUM_MISO,
        .sclk_io_num = PIN_NUM_CLK,
        .quadwp_io_num = -1,
        .quadhd_io_num = -1,
        .max_transfer_sz = 4000,
    };

    // Inicjalizacja szyny SPI
    ret = spi_bus_initialize(host.slot, &bus_cfg, SDSPI_DEFAULT_DMA);
    if (ret != ESP_OK) {
        ESP_LOGE(SD_TAG, "Failed to initialize bus.");
        return ESP_FAIL;
    }

    // Inicjalizacja urządzenia obsługi karty SD dla komunikacji po SPI
    sdspi_device_config_t slot_config = SDSPI_DEVICE_CONFIG_DEFAULT();
    slot_config.gpio_cs = PIN_NUM_CS;
    slot_config.host_id = host.slot;

    // Montowanie systemu plików
    ESP_LOGI(SD_TAG, "Mounting filesystem");
    ret = esp_vfs_fat_sdspi_mount(mount_point, &host, &slot_config, &mount_config, card);

    if (ret != ESP_OK) {
        if (ret == ESP_FAIL) {
            ESP_LOGE(SD_TAG, "Failed to mount filesystem. "
                     "If you want the card to be formatted, set the CONFIG_EXAMPLE_FORMAT_IF_MOUNT_FAILED menuconfig option.");
        } else {
            ESP_LOGE(SD_TAG, "Failed to initialize the card (%s). "
                     "Make sure SD card lines have pull-up resistors in place.", esp_err_to_name(ret));
        }
        return ret;
    }
    ESP_LOGI(SD_TAG, "Filesystem mounted");
    // Print all info about cards
    sdmmc_card_print_info(stdout, *card);

    // Inicjalizację zakończono pomyślnie
    return ESP_OK;
}

uint16_t count_instructions(FILE *f){
    uint16_t lines = 0;
    while(!feof(f)){
        char ch = fgetc(f);
        if(ch == '\n'){
            lines++;
        }
    }
    return lines;
}

esp_err_t sd_write_file(const char *path, char *data)
{
    ESP_LOGI(SD_TAG, "Opening file %s", path);
    FILE *f = fopen(path, "a");
    if (f == NULL) {
        ESP_LOGE(SD_TAG, "Failed to open file for writing");
        return ESP_FAIL;
    }
    
    fprintf(f, "%s", data);
    fclose(f);
    ESP_LOGI(SD_TAG, "File written");

    return ESP_OK;
}

esp_err_t sd_read_file(const char *path, void (*func)(FILE*))
{
    ESP_LOGI(SD_TAG, "Reading file %s", path);
    FILE *f = fopen(path, "r");
    if (f == NULL) {
        ESP_LOGE(SD_TAG, "Failed to open file for reading");
        return ESP_FAIL;
    }
    
    fseek(f, 0, SEEK_SET);
    func(f);
    fclose(f);

    return ESP_OK;
}

//--- IGNORE ---
// void free_resources(const char mount_point[], sdmmc_host_t host, sdmmc_card_t *card){
//     esp_vfs_fat_sdcard_unmount(mount_point, card);
//     ESP_LOGI(SD_TAG, "Card unmounted");

//     //deinitialize the bus after all devices are removed
//     spi_bus_free(host.slot);
// }
