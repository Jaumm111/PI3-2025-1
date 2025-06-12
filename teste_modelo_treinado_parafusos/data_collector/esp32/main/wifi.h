#ifndef __WIFI_H__
#define __WIFI_H__

#include <cstring>

#include "esp_log.h"
#include "esp_system.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_netif.h"

#include "nvs_flash.h"

#define EXAMPLE_WIFI_SSID CONFIG_ESP_WIFI_SSID
#define EXAMPLE_WIFI_PASSWORD CONFIG_ESP_WIFI_PASSWORD

#define EXAMPLE_WIFI_MAXIMUM_RETRY 5
#define ESP_WIFI_SAE_MODE WPA3_SAE_PWE_BOTH
#define EXAMPLE_H2E_IDENTIFIER ""
#define ESP_WIFI_SCAN_AUTH_MODE_THRESHOLD WIFI_AUTH_WPA2_PSK

/* The event group allows multiple bits for each event, but we only care about two events:
 * - we are connected to the AP with an IP
 * - we failed to connect after the maximum amount of retries */
#define WIFI_CONNECTED_BIT BIT0
#define WIFI_FAIL_BIT      BIT1


class Wifi {
    private:
        bool m_connected;
    public:
        Wifi();
        ~Wifi() = default;

        void connect();
        void disconnect();
        void showRSSI();

        bool isConnected();
};
#endif
