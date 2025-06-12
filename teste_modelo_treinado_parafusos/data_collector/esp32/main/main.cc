#include "wifi.h"
#include "stream_server.h"

static const char *TAG = "MAIN";

void wifi_task(void) {
    Wifi wifi;

    while (1) {
        vTaskDelay(5000 / portTICK_PERIOD_MS);

        if (!wifi.isConnected()) {
            ESP_LOGE(TAG, "Failed to connect to the WiFi network");
        } else {
            ESP_LOGI(TAG, "Connected to the WiFi network");
            wifi.showRSSI();
        }
    }
    wifi.disconnect();
}

void mainTask(void) {
    Wifi wifi;
    StreamServer streamServer;

    if (!wifi.isConnected()) {
        ESP_LOGE(TAG, "Failed to connect to the WiFi network");
        return ;
    }

    streamServer.Start();
    while (1) {
    }
}


extern "C" void app_main() {
    /* Initialize NVS — it is used to store PHY calibration data */
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK( ret );

//     mainTask();
    xTaskCreate((TaskFunction_t)&mainTask, "mainTask", 4 * 1024, NULL, tskIDLE_PRIORITY, NULL);
    vTaskDelete(NULL);
}
