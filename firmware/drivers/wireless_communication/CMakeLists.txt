set(srcs)
set(include_dirs ".")

list(APPEND srcs "wireless_communication.h" "wireless_communication.c")

idf_component_register(SRCS "${srcs}"
                       INCLUDE_DIRS "${include_dirs}"
                       REQUIRES esp_wifi esp_http_server nvs_flash)