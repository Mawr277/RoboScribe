/*
 * SPDX-FileCopyrightText: 2022 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: Unlicense OR CC0-1.0
 */
/* HTTP File Server Example

   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/

#include <stdio.h>
#include <string.h>
#include <sys/param.h>
#include <sys/unistd.h>
#include <sys/stat.h>
#include <dirent.h>

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

/* Scratch buffer size */
#define SCRATCH_BUFSIZE  8192

#define DATA_BUFSIZE 128

struct file_server_data {
    /* Base path of file storage */
    char base_path[ESP_VFS_PATH_MAX + 1];

    /* Scratch buffer for temporary storage during file transfer */
    char scratch[SCRATCH_BUFSIZE];
};

static const char *TAG = "file_server";

/* Send HTTP response with a run-time generated html consisting of
 * a list of all files and folders under the requested path.
 * In case of SPIFFS this returns empty list when path is any
 * string other than '/', since SPIFFS doesn't support directories */
static esp_err_t http_resp_dir(httpd_req_t *req, const char *dirpath)
{
    char entrypath[FILE_PATH_MAX];
    char entrysize[16];
    const char *entrytype;

    struct dirent *entry;
    struct stat entry_stat;

    DIR *dir = opendir(dirpath);
    const size_t dirpath_len = strlen(dirpath);

    /* Retrieve the base path of file storage to construct the full path */
    strlcpy(entrypath, dirpath, sizeof(entrypath));

    if (!dir) {
        ESP_LOGE(TAG, "Failed to stat dir : %s", dirpath);
        /* Respond with 404 Not Found */
        httpd_resp_send_err(req, HTTPD_404_NOT_FOUND, "Directory does not exist");
        return ESP_FAIL;
    }

    while ((entry = readdir(dir)) != NULL) {
        entrytype = (entry->d_type == DT_DIR ? "directory" : "file");

        strlcpy(entrypath + dirpath_len, entry->d_name, sizeof(entrypath) - dirpath_len);
        if (stat(entrypath, &entry_stat) == -1) {
            ESP_LOGE(TAG, "Failed to stat %s : %s", entrytype, entry->d_name);
            continue;
        }
        sprintf(entrysize, "%ld", entry_stat.st_size);
        ESP_LOGI(TAG, "Found %s : %s (%s bytes)", entrytype, entry->d_name, entrysize);
    }
    closedir(dir);

    httpd_resp_sendstr_chunk(req, NULL);
    return ESP_OK;
} 

#define IS_FILE_EXT(filename, ext) \
    (strcasecmp(&filename[strlen(filename) - sizeof(ext) + 1], ext) == 0)

/* Set HTTP response content type according to file extension */
static esp_err_t set_content_type_from_file(httpd_req_t *req, const char *filename)
{
    if (IS_FILE_EXT(filename, ".pdf")) {
        return httpd_resp_set_type(req, "application/pdf");
    } else if (IS_FILE_EXT(filename, ".html")) {
        return httpd_resp_set_type(req, "text/html");
    } else if (IS_FILE_EXT(filename, ".jpeg")) {
        return httpd_resp_set_type(req, "image/jpeg");
    } else if (IS_FILE_EXT(filename, ".ico")) {
        return httpd_resp_set_type(req, "image/x-icon");
    }else if (IS_FILE_EXT(filename, ".gcode")) {
        return httpd_resp_set_type(req, "application/octet-stream");
    }
    /* This is a limited set only */
    /* For any other type always set as plain text */
    return httpd_resp_set_type(req, "text/plain");
}

/* Copies the full path into destination buffer and returns
 * pointer to path (skipping the preceding base path) */
static const char* get_path_from_uri(char *dest, const char *base_path, const char *uri, size_t destsize)
{
    const size_t base_pathlen = strlen(base_path);
    size_t pathlen = strlen(uri);

    const char *quest = strchr(uri, '?');
    if (quest) {
        pathlen = MIN(pathlen, quest - uri);
    }
    const char *hash = strchr(uri, '#');
    if (hash) {
        pathlen = MIN(pathlen, hash - uri);
    }

    if (base_pathlen + pathlen + 1 > destsize) {
        /* Full path string won't fit into destination buffer */
        return NULL;
    }

    /* Construct full path (base + path) */
    strcpy(dest, base_path);
    strlcpy(dest + base_pathlen, uri, pathlen + 1);

    /* Return pointer to path, skipping the base */
    return dest + base_pathlen;
}

esp_err_t state_post_handler(httpd_req_t *req) {
    char buf[DATA_BUFSIZE];
    int size;

    // Zabezpieczenie przed przepełnieniem bufora
    if (req->content_len >= sizeof(buf)){
        httpd_resp_send_err(req, HTTPD_400_BAD_REQUEST, "Payload too large");
        return ESP_FAIL;
    }

    size = httpd_req_recv(req, buf, req->content_len);
    if (size <= 0) {
        if (size == HTTPD_SOCK_ERR_TIMEOUT) {
            httpd_resp_send_408(req);
        }
        return ESP_FAIL;
    }

    buf[size] = '\0';
    robot_state cmd = STOP;
    uint8_t status_code;
    static robot_state prev_state = STOP;

    if (xSemaphoreTake(robot_state_mutex, pdMS_TO_TICKS(500))){
        //TODO: Edit to use only one xSemaphoreGive
        uint8_t parsed = sscanf(buf, "statusCode=%hhu&filename=%63s", &status_code, filename);
        if (parsed >= 1) {
            cmd = (robot_state)status_code;
            // Overflow if status_code is bigger than char
            
            if (cmd == START) {
                if (parsed != 2) {
                    httpd_resp_send_err(req, HTTPD_400_BAD_REQUEST, "Missing filename");
                    xSemaphoreGive(robot_state_mutex);
                    return ESP_FAIL;
                }
                
                size_t len = strlen(filename);
                if (len < 6 || strcmp(filename + len - 6, ".gcode") != 0) {
                    httpd_resp_send_err(req, HTTPD_400_BAD_REQUEST, "Invalid file type");
                    xSemaphoreGive(robot_state_mutex);
                    return ESP_FAIL;
                }

                current_state = START;
                ESP_LOGI(TAG, "START file: %s", filename);
            } else if (cmd == STOP) {
                current_state = STOP;
                ESP_LOGI(TAG, "STOP");
            } else if (cmd == PAUSE && prev_state == START){
                current_state = PAUSE;
                ESP_LOGI(TAG, "PAUSE");
            } else {
                ESP_LOGW(TAG, "Unknown command");
            }

            prev_state = cmd;
            httpd_resp_sendstr(req, "State updated successfully");
            xSemaphoreGive(robot_state_mutex);
            return ESP_OK;
        }
        xSemaphoreGive(robot_state_mutex);
    } else {
        httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, "Mutex timeout");
        return ESP_FAIL;
    }

    httpd_resp_send_err(req, HTTPD_400_BAD_REQUEST, "Invalid state data");
    return ESP_FAIL;
}

esp_err_t manual_control_post_handler(httpd_req_t *req) {
    /// Variables for angles: base, arm, tool, and coordinates: x, y
    mov_controls ctrl = {0};
    char buf[DATA_BUFSIZE];
    int size;
    uint8_t res;

    if (req->content_len >= sizeof(buf)) {
        httpd_resp_send_err(req, HTTPD_400_BAD_REQUEST, "Payload too large");
        return ESP_FAIL;
    }

    size = httpd_req_recv(req, buf, req->content_len);
    if (size <= 0){
        if(size == HTTPD_SOCK_ERR_TIMEOUT) {
            httpd_resp_send_408(req);
        }
        return ESP_FAIL;
    }

    buf[size] = '\0';
    res = sscanf(buf, "base_angle=%hhu&arm_angle=%hhu&tool_angle=%hhu&x_coord=%hhu&y_coord=%hhu", 
        &ctrl.base_angle, 
        &ctrl.arm_angle, 
        &ctrl.tool_angle, 
        &ctrl.x_coord, 
        &ctrl.y_coord
    );

    if (res == 5){
        xQueueSend(manual_mov_queue, &ctrl, pdMS_TO_TICKS(100));

        ESP_LOGI(TAG, "Parsed successfully: base=%d, arm=%d, tool=%d, x=%d, y=%d",
            ctrl.base_angle, 
            ctrl.arm_angle, 
            ctrl.tool_angle, 
            ctrl.x_coord, 
            ctrl.y_coord
        );
        
        httpd_resp_sendstr(req, "Position data acquired sucessfully");
        return ESP_OK;
    }
    
    httpd_resp_send_err(req, HTTPD_400_BAD_REQUEST, "Invalid data");
    return ESP_FAIL;
}

esp_err_t accel_vector_get_handler(httpd_req_t *req){
    accel_vector *data = (accel_vector*)req->user_ctx;
    if (data == NULL) {
        httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, "Missing data");
        return ESP_FAIL;
    }

    char buf[DATA_BUFSIZE];

    if(xSemaphoreTake(imu_readings_mutex, pdMS_TO_TICKS(200))){
        snprintf(buf, sizeof(buf), "x_accel=%hd&y_accel=%hd&z_accel=%hd",
            data->x_accel, 
            data->y_accel, 
            data->z_accel
        );
        xSemaphoreGive(imu_readings_mutex);
    } else {
        httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, "Mutex timeout");
        return ESP_FAIL;
    }

    ESP_LOGI(TAG, "Sent successfully: %s", buf);

#ifdef CONFIG_EXAMPLE_HTTPD_CONN_CLOSE_HEADER
    httpd_resp_set_hdr(req, "Connection", "close");
#endif
    httpd_resp_send(req, buf, HTTPD_RESP_USE_STRLEN);
    
    return ESP_OK;
}

/* Handler to download a file kept on the server */
static esp_err_t download_get_handler(httpd_req_t *req)
{
    char filepath[FILE_PATH_MAX];
    FILE *fd = NULL;
    struct stat file_stat;

    const char *filename = get_path_from_uri(filepath, ((struct file_server_data *)req->user_ctx)->base_path,
                                             req->uri, sizeof(filepath));
    if (!filename) {
        ESP_LOGE(TAG, "Filename is too long");
        /* Respond with 500 Internal Server Error */
        httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, "Filename too long");
        return ESP_FAIL;
    }

    /* If name has trailing '/', respond with directory contents */
    if (filename[strlen(filename) - 1] == '/') {
        return http_resp_dir(req, filepath);
    }

    if (stat(filepath, &file_stat) == -1) {
        ESP_LOGE(TAG, "Failed to stat file : %s", filepath);
        /* Respond with 404 Not Found */
        httpd_resp_send_err(req, HTTPD_404_NOT_FOUND, "File does not exist");
        return ESP_FAIL;
    }

    if (xSemaphoreTake(sd_card_mutex, pdMS_TO_TICKS(portMAX_DELAY))){
        fd = fopen(filepath, "r");
        if (!fd) {
            ESP_LOGE(TAG, "Failed to read existing file : %s", filepath);
            /* Respond with 500 Internal Server Error */
            httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, "Failed to read existing file");
            xSemaphoreGive(sd_card_mutex);
            return ESP_FAIL;
        }

        ESP_LOGI(TAG, "Sending file : %s (%ld bytes)...", filename, file_stat.st_size);
        set_content_type_from_file(req, filename);

        /* Retrieve the pointer to scratch buffer for temporary storage */
        char *chunk = ((struct file_server_data *)req->user_ctx)->scratch;
        size_t chunksize;
        do {
            /* Read file in chunks into the scratch buffer */
            chunksize = fread(chunk, 1, SCRATCH_BUFSIZE, fd);

            if (chunksize > 0) {
                /* Send the buffer contents as HTTP response chunk */
                if (httpd_resp_send_chunk(req, chunk, chunksize) != ESP_OK) {
                    fclose(fd);
                    ESP_LOGE(TAG, "File sending failed!");
                    /* Abort sending file */
                    httpd_resp_sendstr_chunk(req, NULL);
                    /* Respond with 500 Internal Server Error */
                    httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, "Failed to send file");
                    xSemaphoreGive(sd_card_mutex);
                return ESP_FAIL;
                }
            }

            /* Keep looping till the whole file is sent */
        } while (chunksize != 0);

        /* Close file after sending complete */
        fclose(fd);
        xSemaphoreGive(sd_card_mutex);
    }
    ESP_LOGI(TAG, "File sending complete");

    /* Respond with an empty chunk to signal HTTP response completion */
#ifdef CONFIG_EXAMPLE_HTTPD_CONN_CLOSE_HEADER
    httpd_resp_set_hdr(req, "Connection", "close");
#endif
    httpd_resp_send_chunk(req, NULL, 0);
    return ESP_OK;
}

/* Handler to upload a file onto the server */
static esp_err_t upload_post_handler(httpd_req_t *req)
{
    char filepath[FILE_PATH_MAX];
    FILE *fd = NULL;
    struct stat file_stat;

    /* Skip leading "/upload" from URI to get filename */
    /* Note sizeof() counts NULL termination hence the -1 */
    const char *filename = get_path_from_uri(filepath, ((struct file_server_data *)req->user_ctx)->base_path,
                                             req->uri + sizeof("/upload") - 1, sizeof(filepath));
    if (!filename) {
        /* Respond with 500 Internal Server Error */
        httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, "Filename too long");
        return ESP_FAIL;
    }

    /* Filename cannot have a trailing '/' */
    if (filename[strlen(filename) - 1] == '/') {
        ESP_LOGE(TAG, "Invalid filename : %s", filename);
        httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, "Invalid filename");
        return ESP_FAIL;
    }

    if (stat(filepath, &file_stat) == 0) {
        ESP_LOGE(TAG, "File already exists : %s", filepath);
        /* Respond with 400 Bad Request */
        httpd_resp_send_err(req, HTTPD_400_BAD_REQUEST, "File already exists");
        return ESP_FAIL;
    }

    /* File cannot be larger than a limit */
    if (req->content_len > MAX_FILE_SIZE) {
        ESP_LOGE(TAG, "File too large : %d bytes", req->content_len);
        /* Respond with 400 Bad Request */
        httpd_resp_send_err(req, HTTPD_400_BAD_REQUEST,
                            "File size must be less than "
                            MAX_FILE_SIZE_STR "!");
        /* Return failure to close underlying connection else the
         * incoming file content will keep the socket busy */
        return ESP_FAIL;
    }

    if (xSemaphoreTake(sd_card_mutex, pdMS_TO_TICKS(portMAX_DELAY))){
        fd = fopen(filepath, "w");
        if (!fd) {
            ESP_LOGE(TAG, "Failed to create file : %s", filepath);
            /* Respond with 500 Internal Server Error */
            httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, "Failed to create file");
            xSemaphoreGive(sd_card_mutex);
            return ESP_FAIL;
        }

        ESP_LOGI(TAG, "Receiving file : %s...", filename);

        /* Retrieve the pointer to scratch buffer for temporary storage */
        char *buf = ((struct file_server_data *)req->user_ctx)->scratch;
        int received;

        /* Content length of the request gives
        * the size of the file being uploaded */
        int remaining = req->content_len;

        while (remaining > 0) {

            ESP_LOGI(TAG, "Remaining size : %d", remaining);
            /* Receive the file part by part into a buffer */
            if ((received = httpd_req_recv(req, buf, MIN(remaining, SCRATCH_BUFSIZE))) <= 0) {
                if (received == HTTPD_SOCK_ERR_TIMEOUT) {
                    /* Retry if timeout occurred */
                    continue;
                }

                /* In case of unrecoverable error,
                * close and delete the unfinished file*/
                fclose(fd);
                unlink(filepath);
                xSemaphoreGive(sd_card_mutex);

                ESP_LOGE(TAG, "File reception failed!");
                /* Respond with 500 Internal Server Error */
                httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, "Failed to receive file");
                return ESP_FAIL;
            }

            /* Write buffer content to file on storage */
            if (received && (received != fwrite(buf, 1, received, fd))) {
                /* Couldn't write everything to file!
                * Storage may be full? */
                fclose(fd);
                unlink(filepath);
                xSemaphoreGive(sd_card_mutex);

                ESP_LOGE(TAG, "File write failed!");
                /* Respond with 500 Internal Server Error */
                httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, "Failed to write file to storage");
                return ESP_FAIL;
            }

            /* Keep track of remaining size of
            * the file left to be uploaded */
            remaining -= received;
        }

        /* Close file upon upload completion */
        fclose(fd);
        xSemaphoreGive(sd_card_mutex);
    }
    ESP_LOGI(TAG, "File reception complete");

    /* Redirect onto root to see the updated file list */
    httpd_resp_set_status(req, "303 See Other");
    httpd_resp_set_hdr(req, "Location", "/");
#ifdef CONFIG_EXAMPLE_HTTPD_CONN_CLOSE_HEADER
    httpd_resp_set_hdr(req, "Connection", "close");
#endif
    httpd_resp_sendstr(req, "File uploaded successfully");
    return ESP_OK;
}

/* Handler to delete a file from the server */
static esp_err_t delete_post_handler(httpd_req_t *req)
{
    char filepath[FILE_PATH_MAX];
    struct stat file_stat;

    /* Skip leading "/delete" from URI to get filename */
    /* Note sizeof() counts NULL termination hence the -1 */
    const char *filename = get_path_from_uri(filepath, ((struct file_server_data *)req->user_ctx)->base_path,
                                             req->uri  + sizeof("/delete") - 1, sizeof(filepath));
    if (!filename) {
        /* Respond with 500 Internal Server Error */
        httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, "Filename too long");
        return ESP_FAIL;
    }

    /* Filename cannot have a trailing '/' */
    if (filename[strlen(filename) - 1] == '/') {
        ESP_LOGE(TAG, "Invalid filename : %s", filename);
        httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, "Invalid filename");
        return ESP_FAIL;
    }

    if (stat(filepath, &file_stat) == -1) {
        ESP_LOGE(TAG, "File does not exist : %s", filename);
        /* Respond with 400 Bad Request */
        httpd_resp_send_err(req, HTTPD_400_BAD_REQUEST, "File does not exist");
        return ESP_FAIL;
    }

    ESP_LOGI(TAG, "Deleting file : %s", filename);
    /* Delete file */
    if(xSemaphoreTake(sd_card_mutex, pdMS_TO_TICKS(portMAX_DELAY))){
        unlink(filepath);
        xSemaphoreGive(sd_card_mutex);
    }

    /* Redirect onto root to see the updated file list */
    httpd_resp_set_status(req, "303 See Other");
    httpd_resp_set_hdr(req, "Location", "/");
#ifdef CONFIG_EXAMPLE_HTTPD_CONN_CLOSE_HEADER
    httpd_resp_set_hdr(req, "Connection", "close");
#endif
    httpd_resp_sendstr(req, "File deleted successfully");
    return ESP_OK;
}

/* Function to start the file server */
esp_err_t start_file_server(const char *base_path)
{
    static struct file_server_data *server_data = NULL;

    if (server_data) {
        ESP_LOGE(TAG, "File server already started");
        return ESP_ERR_INVALID_STATE;
    }

    /* Allocate memory for server data */
    server_data = calloc(1, sizeof(struct file_server_data));
    if (!server_data) {
        ESP_LOGE(TAG, "Failed to allocate memory for server data");
        return ESP_ERR_NO_MEM;
    }
    strlcpy(server_data->base_path, base_path,
            sizeof(server_data->base_path));

    httpd_handle_t server = NULL;
    httpd_config_t config = HTTPD_DEFAULT_CONFIG();

    /* Use the URI wildcard matching function in order to
     * allow the same handler to respond to multiple different
     * target URIs which match the wildcard scheme */
    config.uri_match_fn = httpd_uri_match_wildcard;

    ESP_LOGI(TAG, "Starting HTTP Server on port: '%d'", config.server_port);
    if (httpd_start(&server, &config) != ESP_OK) {
        ESP_LOGE(TAG, "Failed to start file server!");
        return ESP_FAIL;
    }

    /* URI handler for getting uploaded files */
    httpd_uri_t get_data = {
        .uri       = "/accel_data",
        .method    = HTTP_GET,
        .handler   = accel_vector_get_handler,
        .user_ctx  = &imu_readings
    };
    httpd_register_uri_handler(server, &get_data);

    httpd_uri_t file_download = {
        .uri       = "/*",  // Match all URIs of type /path/to/file
        .method    = HTTP_GET,
        .handler   = download_get_handler,
        .user_ctx  = server_data    // Pass server data as context
    };
    httpd_register_uri_handler(server, &file_download);

    httpd_uri_t set_state = {
        .uri       = "/state",
        .method    = HTTP_POST,
        .handler   = state_post_handler,
        .user_ctx  = server_data
    };
    httpd_register_uri_handler(server, &set_state);

    httpd_uri_t set_mov_controls = {
        .uri       = "/controls",
        .method    = HTTP_POST,
        .handler   = manual_control_post_handler,
        .user_ctx  = server_data
    };
    httpd_register_uri_handler(server, &set_mov_controls);

    /* URI handler for uploading files to server */
    httpd_uri_t file_upload = {
        .uri       = "/upload/*",   // Match all URIs of type /upload/path/to/file
        .method    = HTTP_POST,
        .handler   = upload_post_handler,
        .user_ctx  = server_data    // Pass server data as context
    };
    httpd_register_uri_handler(server, &file_upload);

    /* URI handler for deleting files from server */
    httpd_uri_t file_delete = {
        .uri       = "/delete/*",   // Match all URIs of type /delete/path/to/file
        .method    = HTTP_POST,
        .handler   = delete_post_handler,
        .user_ctx  = server_data    // Pass server data as context
    };
    httpd_register_uri_handler(server, &file_delete);

    return ESP_OK;
}
