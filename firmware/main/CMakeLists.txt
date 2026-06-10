idf_component_register(SRCS "main.c"
                       INCLUDE_DIRS "."
                       REQUIRES freertos esp_timer drivers esp_netif nvs_flash esp_event protocol_examples_common)