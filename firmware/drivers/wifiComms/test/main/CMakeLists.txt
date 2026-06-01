idf_component_register(SRCS "main.c"
                       PRIV_REQUIRES SDcard nvs_flash esp_netif esp_event protocol_examples_common wifiComms)