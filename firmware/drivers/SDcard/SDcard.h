/**
 * @file SDcard.h
 * @author Maciej Nowicki (maciejnowicki1234@gmail.com)
 * @brief Plik nagłówkowy programu do obsługi zapisywania i czytania 
 * z karty SD stosując interfejs SPI.
 * @version 1.0
 * @date 2026-01-13
 * 
 * @copyright Copyright (c) 2026
 * 
 */

#include <string.h>
#include <sys/unistd.h>
#include <sys/stat.h>
#include "sdkconfig.h"
#include "esp_vfs_fat.h"
#include "sdmmc_cmd.h"
#include "esp_err.h"

/**
 * @def MAX_CHAR_SIZE 
 * @brief Maksymalna długość znaków 1 linijki kodu
 */
#define MAX_CHAR_SIZE    64

/**
 * @def MOUNT_POINT
 * @brief ścieżka pod jaką dołączyć pliki karty SD
 */
#define MOUNT_POINT "/sdcard"

#define PIN_NUM_MISO  CONFIG_PIN_MISO
#define PIN_NUM_MOSI  CONFIG_PIN_MOSI
#define PIN_NUM_CLK   CONFIG_PIN_CLK
#define PIN_NUM_CS    CONFIG_PIN_CS

/**
 * @brief Inicjalizuje parametry obsługi karty SD
 * 
 * Do zamontowania karty SD musimy skonfigurować system plików karty SD,
 * stworzyć uchwyt do hosta SDSPI, zainicjalizować szynę SPI
 * i slot obsługi karty SD, po czym możemy zamontować do system plików 
 * karty SD do systemu MCU.
 * @param card wskaźnik do wskaźnika wskazującego uchwyt obsługi karty SD
 * @return esp_err_t 
 */
esp_err_t card_init(sdmmc_card_t **card);

/**
 * @brief Liczy ile instrukcji posiada program
 * 
 * @param f wskaźnik na uchwyt do pliku
 * @return uint16_t liczba instrukcji w kodzie
 */
uint16_t count_instructions(FILE *f);

/**
 * @brief Dodaj tekst ze zmiennej data do pliku
 * 
 * @note Funkcja zapisuje w trybie append, więc należy wyczyścić plik wcześniej
 * @param path ścieżka do pliku z programem
 * @param data dane do zapisu
 * @return esp_err_t 
 */
esp_err_t sd_write_file(const char *path, char *data);

/**
 * @brief 
 * 
 * @param path ścieżka do pliku z programem
 * @param commands_num liczba poleceń kodu w pliku
 * @param func wskaźnik do funkcji w której wykonywany będzie kod programu
 * @return esp_err_t 
 */
esp_err_t sd_read_file(const char *path, void (*func)(FILE*));
//esp_err_t sd_deinit(sdmmc_card_t *card); //--- IGNORE ---
