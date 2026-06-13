/**
 * @file file_server.h
 * @author Maciej Nowicki (maciejnowicki1234@gmail.com)
 * @brief Plik nagłówkowy implementacji serwera HTTP
 * @version 0.1
 * @date 2026-06-10
 * @ingroup wirelessComms
 * 
 * @copyright Copyright (c) 2026
 * 
 */
#include <stdio.h>
#include <string.h>
#include <sys/param.h>
#include <sys/unistd.h>
#include <sys/stat.h>
#include <dirent.h>

#include "sdkconfig.h"
#include "esp_err.h"
#include "esp_log.h"
#include "esp_vfs.h"
#include "esp_http_server.h"

#include "robot_data.h"

/* Max length a file path can have on storage */
#define FILE_PATH_MAX (ESP_VFS_PATH_MAX + CONFIG_SPIFFS_OBJ_NAME_LEN)

/* Max size of an individual file. Make sure this
 * value is same as that set in upload_script.html */
#define MAX_FILE_SIZE   (200*1024) // 200 KB
#define MAX_FILE_SIZE_STR "200KB"

/// Scratch buffer size
#define SCRATCH_BUFSIZE  8192
#define DATA_BUFSIZE 128

/// @brief Struktura danych 
struct file_server_data {
   /// Base path of file storage
   char base_path[ESP_VFS_PATH_MAX + 1];

   /// Scratch buffer for temporary storage during file transfer
   char scratch[SCRATCH_BUFSIZE];
};

/**
 * @brief Inicjalizuje i uruchamia serwer plików HTTP.
 * 
 * Funkcja konfiguruje serwer, rejestruje odpowiednie procedury obsługi żądań HTTP 
 * dla operacji pobierania/wgrywania/usuwania plików, a także zarządzania stanem robota.
 * 
 * @param base_path Podstawowa ścieżka (mount point) dla systemu plików.
 * @return esp_err_t ESP_OK w przypadku poprawnego uruchomienia, kod błędu w przypadku problemów.
 */
esp_err_t start_file_server(const char *base_path);

