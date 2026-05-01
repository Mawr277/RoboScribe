#include "wireless_communication.h"

esp_err_t upload_post_handler(httpd_req_t *req){
    const char *filepath = MOUNT_POINT"/program.txt";
    FILE *fd = NULL;

    char buf[MAX_HTTP_RECV];
    int received;
    int remaining = req->content_len;

    fd = fopen(filepath, "w");
    if (!fd) {
        ESP_LOGE(TAG, "Failed to create file : %s", filepath);
        /* Respond with 500 Internal Server Error */
        httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, "Failed to create file");
        return ESP_FAIL;
    }

    fclose(fd);

    ESP_LOGI(TAG, "Receiving file : %s...", filename);

    while(remaining >0){
        ESP_LOGI(TAG, "Remaining size : %d", remaining);
        if((received = httpd_req_recv(req,buf,MIN(remaining,sizeof(buf))))<=0){
            if (received == HTTPD_SOCK_ERR_TIMEOUT){
                continue;
            }

            /* In case of unrecoverable error,
             * close and delete the unfinished file*/
            unlink(filepath);
            ESP_LOGE(TAG, "File reception failed!");
            /* Respond with 500 Internal Server Error */
            httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, "Failed to receive file");
            return ESP_FAIL;

        }
        
        if (received < sizeof(buf)) buf[received] = '\0';

        sd_write_file(filepath,buf);
        remaining -= received;
    }

    httpd_resp_sendstr(req, "File uploaded successfully");
    return ESP_OK;
}