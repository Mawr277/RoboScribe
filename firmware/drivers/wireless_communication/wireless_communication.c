#include "wireless_communication.h"

esp_err_t upload_post_handler(httpd_req_t *req){
    const char *filepath = MOUNT_POINT"/program.gcode";
    FILE *fd = NULL;

    char buf[MAX_HTTP_RECV]
    int received;
    int remaining = req->content_len;

    fd = fopen(filepath, "w");
    if (!fd) {
        ESP_LOGE(TAG, "Failed to create file : %s", filepath);
        /* Respond with 500 Internal Server Error */
        httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, "Failed to create file");
        return ESP_FAIL;
    }

}