idf_component_register(SRCS "take_picture.c"
                        PRIV_INCLUDE_DIRS "."
                        PRIV_REQUIRES nvs_flash esp_psram
                        REQUIRES esp_http_server esp_timer esp_wifi esp_event esp_http_client) 
