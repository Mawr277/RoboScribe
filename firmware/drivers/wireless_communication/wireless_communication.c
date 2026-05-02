#include "wireless_communication.h"
#include <sys/param.h>

#define MAXIMUM_RETRY 5
static int s_retry_num = 0;

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

    ESP_LOGI(WIRELESS_COMMUNICATION_TAG, "Starting HTTP Server on port: '%d'", config.server_port);
    if (httpd_start(&server, &config) != ESP_OK) {
        ESP_LOGE(WIRELESS_COMMUNICATION_TAG, "Failed to start file server!");
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


static void wifi_event_handler(void* arg, esp_event_base_t event_base,
                               int32_t event_id, void* event_data) {
    if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START) {
        // Zdarzenie: Wi-Fi wystartowało, próbujemy się połączyć
        esp_wifi_connect();
        ESP_LOGI(WIRELESS_COMMUNICATION_TAG, "Connecting to AP...");
    } else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED) {
        // Zdarzenie: Rozłączono z Wi-Fi. Próbujemy połączyć się ponownie.
        if (s_retry_num < MAXIMUM_RETRY) {
            esp_wifi_connect();
            s_retry_num++;
            ESP_LOGI(WIRELESS_COMMUNICATION_TAG, "Retry to connect to the AP");
        } else {
            ESP_LOGE(WIRELESS_COMMUNICATION_TAG, "Connection to the AP failed");
        }
    } else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP) {
        // Zdarzenie: Pomyślnie uzyskano adres IP z serwera DHCP
        ip_event_got_ip_t* event = (ip_event_got_ip_t*) event_data;
        ESP_LOGI(WIRELESS_COMMUNICATION_TAG, "Got IP:" IPSTR, IP2STR(&event->ip_info.ip));
        s_retry_num = 0; // Resetujemy licznik prób po udanym połączeniu
    }
}

void wifi_init_sta(void) {
    // 1. Inicjalizacja podstawowego stosu sieciowego (TCP/IP)
    ESP_ERROR_CHECK(esp_netif_init());

    // 2. Utworzenie domyślnej pętli zdarzeń systemowych
    ESP_ERROR_CHECK(esp_event_loop_create_default());
    esp_netif_create_default_wifi_sta();

    // 3. Inicjalizacja Wi-Fi z domyślną konfiguracją
    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));

    // 4. Rejestracja obsługi zdarzeń (Event Handlers)
    esp_event_handler_instance_t instance_any_id;
    esp_event_handler_instance_t instance_got_ip;
    ESP_ERROR_CHECK(esp_event_handler_instance_register(WIFI_EVENT,
                                                        ESP_EVENT_ANY_ID,
                                                        &wifi_event_handler,
                                                        NULL,
                                                        &instance_any_id));
    ESP_ERROR_CHECK(esp_event_handler_instance_register(IP_EVENT,
                                                        IP_EVENT_STA_GOT_IP,
                                                        &wifi_event_handler,
                                                        NULL,
                                                        &instance_got_ip));

    // 5. Konfiguracja parametrów logowania do sieci używając makr z pliku .h
    wifi_config_t wifi_config = {
        .sta = {
            .ssid = WIFI_SSID,
            .password = WIFI_PASS,
            // Autentykacja WPA2_PSK to bezpieczny standard na dziś.
            .threshold.authmode = WIFI_AUTH_WPA2_PSK,
        },
    };

    // 6. Uruchomienie układu radiowego i zatwierdzenie konfiguracji
    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &wifi_config));
    ESP_ERROR_CHECK(esp_wifi_start());

    ESP_LOGI(WIRELESS_COMMUNICATION_TAG, "wifi_init_sta finished.");
}