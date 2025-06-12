idf_component_register(
    SRCS
        "wifi.cc"
        "stream_server.cc"
        "main.cc"
    PRIV_INCLUDE_DIRS "."
    PRIV_REQUIRES spi_flash esp_wifi wpa_supplicant nvs_flash esp_http_server esp_timer
    INCLUDE_DIRS "")
