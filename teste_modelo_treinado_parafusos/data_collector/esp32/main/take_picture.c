/**
 * This example takes a picture every 5s and print its size on serial monitor.
 */

#include <esp_log.h>
#include <esp_system.h>
#include <nvs_flash.h>
#include <sys/param.h>
#include <string.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

// support IDF 5.x
#ifndef portTICK_RATE_MS
#define portTICK_RATE_MS portTICK_PERIOD_MS
#endif

#include "esp_camera.h"
/*
#define CAM_MODULE_NAME "ESP-EYE"
#define CAM_PIN_PWDN -1
#define CAM_PIN_RESET -1
#define CAM_PIN_XCLK 4
#define CAM_PIN_SIOD 18
#define CAM_PIN_SIOC 23

#define CAM_PIN_D7 36
#define CAM_PIN_D6 37
#define CAM_PIN_D5 38
#define CAM_PIN_D4 39
#define CAM_PIN_D3 35
#define CAM_PIN_D2 14
#define CAM_PIN_D1 13
#define CAM_PIN_D0 34
#define CAM_PIN_VSYNC 5
#define CAM_PIN_HREF 27
#define CAM_PIN_PCLK 25
*/
// Definições da câmera
#define BOARD_ESP32CAM_AITHINKER

// Configuração dos pinos da câmera 
#ifdef BOARD_ESP32CAM_AITHINKER
#define CAM_PIN_PWDN 32
#define CAM_PIN_RESET -1
#define CAM_PIN_XCLK 0
#define CAM_PIN_SIOD 26
#define CAM_PIN_SIOC 27
#define CAM_PIN_D7 35
#define CAM_PIN_D6 34
#define CAM_PIN_D5 39
#define CAM_PIN_D4 36
#define CAM_PIN_D3 21
#define CAM_PIN_D2 19
#define CAM_PIN_D1 18
#define CAM_PIN_D0 5
#define CAM_PIN_VSYNC 25
#define CAM_PIN_HREF 23
#define CAM_PIN_PCLK 22
#endif

static const char *TAG = "example:take_picture";

static camera_config_t camera_config = {
  .ledc_channel = LEDC_CHANNEL_0,
  .ledc_timer = LEDC_TIMER_0,
  .pin_d0 = CAM_PIN_D0,
  .pin_d1 = CAM_PIN_D1,
  .pin_d2 = CAM_PIN_D2,
  .pin_d3 = CAM_PIN_D3,
  .pin_d4 = CAM_PIN_D4,
  .pin_d5 = CAM_PIN_D5,
  .pin_d6 = CAM_PIN_D6,
  .pin_d7 = CAM_PIN_D7,
  .pin_xclk     = CAM_PIN_XCLK,
  .pin_pclk     = CAM_PIN_PCLK,
  .pin_vsync    = CAM_PIN_VSYNC,
  .pin_href     = CAM_PIN_HREF,
  .pin_sscb_sda = CAM_PIN_SIOD,
  .pin_sscb_scl = CAM_PIN_SIOC,
  .pin_pwdn     = CAM_PIN_PWDN,
  .pin_reset    = CAM_PIN_RESET,
  .xclk_freq_hz = 15000000,
  .jpeg_quality = 10,//0-63, for OV series camera sensors, lower number means higher quality
  .fb_count = 1, //When jpeg mode is used, if fb_count more than one, the driver will work in continuous mode.
  .fb_location = CAMERA_FB_IN_DRAM, 
  .pixel_format = PIXFORMAT_GRAYSCALE,
  .frame_size = FRAMESIZE_96X96, //QQVGA-UXGA, For ESP32, do not use sizes above QVGA when not JPEG. The performance of the ESP32-S series has improved a lot, but JPEG mode always gives better frame rates.
  };

static esp_err_t init_camera(void)
{

    gpio_config_t conf;
    conf.mode = GPIO_MODE_INPUT;
    conf.pull_up_en = GPIO_PULLUP_ENABLE;
    conf.pull_down_en = GPIO_PULLDOWN_DISABLE;
    conf.intr_type = GPIO_INTR_DISABLE;
    conf.pin_bit_mask = 1LL << 13;
    gpio_config(&conf);
    conf.pin_bit_mask = 1LL << 14;
    gpio_config(&conf);

    //initialize the camera
    esp_err_t err = esp_camera_init(&camera_config);
    if (err != ESP_OK)
    {
        ESP_LOGE(TAG, "Camera Init Failed");
        return err;
    }

    return ESP_OK;
}
#endif

void app_main(void)
{
#if ESP_CAMERA_SUPPORTED
    if(ESP_OK != init_camera()) {
        return;
    }

    while (1)
    {
        ESP_LOGI(TAG, "Taking picture...");
        camera_fb_t *pic = esp_camera_fb_get();

        // use pic->buf to access the image
        ESP_LOGI(TAG, "Picture taken! Its size was: %zu bytes", pic->len);
        esp_camera_fb_return(pic);

        vTaskDelay(5000 / portTICK_RATE_MS);
    }
#else
    ESP_LOGE(TAG, "Camera support is not available for this chip");
    return;
#endif
}
