#pragma once

#include "esp_wifi.h"
#include "esp_http_server.h"
#include "esp_event.h"
#include "nvs_flash.h"
#include "esp_log.h"
#include "SDcard.h"

#define WIFI_SSID      "WPISZ_NAZWE_SIECI"
#define WIFI_PASS      "WPISZ_HASLO"
#define MAX_HTTP_RECV  1024

static const char *WIRELESS_COMMUNICATION_TAG = "wireless_communication";

esp_err_t upload_post_handler(httpd_req_t *req);

void wifi_init_sta(void);

esp_err_t start_webserver(void);