#include "wireless_communication.h"
#include <sys/param.h>

esp_err_t upload_post_handler(httpd_req_t *req){
    const char *filepath = MOUNT_POINT"/program.txt";
    FILE *fd = NULL;

    char buf[MAX_HTTP_RECV];
    int received;
    int remaining = req->content_len;

    fd = fopen(filepath, "w");
    if (!fd) {
        ESP_LOGE(WIRELESS_COMMUNICATION_TAG , "Failed to create file : %s", filepath);
        /* Respond with 500 Internal Server Error */
        httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, "Failed to create file");
        return ESP_FAIL;
    }
    fclose(fd);


    while(remaining >0){
        ESP_LOGI(WIRELESS_COMMUNICATION_TAG , "Remaining size : %d", remaining);
        if((received = httpd_req_recv(req,buf,MIN(remaining,sizeof(buf))))<=0){
            if (received == HTTPD_SOCK_ERR_TIMEOUT){
                continue;
            }

            /* In case of unrecoverable error,
             * close and delete the unfinished file*/
            unlink(filepath);
            ESP_LOGE(WIRELESS_COMMUNICATION_TAG , "File reception failed!");
            /* Respond with 500 Internal Server Error */
            httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, "Failed to receive file");
            return ESP_FAIL;

        }

        if (received < sizeof(buf)){
            buf[received] = '\0';
        }


        sd_write_file(filepath,buf);
        remaining -= received;
    }

    ESP_LOGI(WIRELESS_COMMUNICATION_TAG, "File received and saved to SD successfully");
    httpd_resp_sendstr(req, "File uploaded successfully");
    return ESP_OK;
}

/* Function to start the file server */
esp_err_t start_webserver(void)
{
    httpd_handle_t server = NULL;
    httpd_config_t config = HTTPD_DEFAULT_CONFIG();

    ESP_LOGI(TAG, "Starting HTTP Server on port: '%d'", config.server_port);
    if (httpd_start(&server, &config) != ESP_OK) {
        ESP_LOGE(TAG, "Failed to start file server!");
        return ESP_FAIL;
    }

    /* URI handler for uploading files to server */
    httpd_uri_t file_upload = {
        .uri       = "/upload",   // Match all URIs of type /upload/path/to/file
        .method    = HTTP_POST,
        .handler   = upload_post_handler,
        .user_ctx  = NULL
    };
    httpd_register_uri_handler(server, &file_upload);

    return ESP_OK;
}
