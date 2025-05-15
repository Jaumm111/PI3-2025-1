#include <stdio.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_log.h"
#include "nvs_flash.h"
#include "esp_http_client.h"
#include "esp_camera.h"

static const char *TAG = "CAMERA";

// Conexão WiFi 
#define WIFI_SSID "Droga eh o braia"
#define WIFI_PASS "88888888"

// URL do servidor que vai receber a imagem
// Para o teste foi feito um script em python(server.py) e usado o IP local do PC
#define SERVER_URL "http://10.0.0.183/upload"

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

camera_config_t camera_config = {
    .pin_pwdn = CAM_PIN_PWDN,
    .pin_reset = CAM_PIN_RESET,
    .pin_xclk = CAM_PIN_XCLK,
    .pin_sccb_sda = CAM_PIN_SIOD,
    .pin_sccb_scl = CAM_PIN_SIOC,
    .pin_d7 = CAM_PIN_D7,
    .pin_d6 = CAM_PIN_D6,
    .pin_d5 = CAM_PIN_D5,
    .pin_d4 = CAM_PIN_D4,
    .pin_d3 = CAM_PIN_D3,
    .pin_d2 = CAM_PIN_D2,
    .pin_d1 = CAM_PIN_D1,
    .pin_d0 = CAM_PIN_D0,
    .pin_vsync = CAM_PIN_VSYNC,
    .pin_href = CAM_PIN_HREF,
    .pin_pclk = CAM_PIN_PCLK,
    .xclk_freq_hz = 20000000,
    .ledc_timer = LEDC_TIMER_0,
    .ledc_channel = LEDC_CHANNEL_0,
    .pixel_format = PIXFORMAT_JPEG,
    .frame_size = FRAMESIZE_XGA,
    .jpeg_quality = 12, //qualidade do JPEG. 0-63 mais proximo de zero maior sera a qualidade. exemplo veio com 12
    .fb_count = 1,
    .fb_location = CAMERA_FB_IN_PSRAM,
    .grab_mode = CAMERA_GRAB_WHEN_EMPTY,
};

// Event handler para Wi-Fi
static void wifi_event_handler(void* arg, esp_event_base_t event_base,
                              int32_t event_id, void* event_data) {
    if (event_id == WIFI_EVENT_STA_START) {
        esp_wifi_connect();
    } else if (event_id == IP_EVENT_STA_GOT_IP) {
        ESP_LOGI(TAG, "Conectado ao Wi-Fi");
    }
}

// Inicializa o Wi-Fi
void wifi_init() {
    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());
    esp_netif_create_default_wifi_sta();

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));

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

    wifi_config_t wifi_config = {
        .sta = {
            .ssid = WIFI_SSID,
            .password = WIFI_PASS,
            .threshold.authmode = WIFI_AUTH_WPA2_PSK,
        },
    };
    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    ESP_ERROR_CHECK(esp_wifi_set_config(ESP_IF_WIFI_STA, &wifi_config));
    ESP_ERROR_CHECK(esp_wifi_start());
}

// Função para enviar imagem via HTTP POST
void send_http_post(uint8_t *image_data, size_t image_len) {
    esp_http_client_config_t config = {
        .url = SERVER_URL,
        .method = HTTP_METHOD_POST,
        .event_handler = NULL,
        .user_data = NULL,
    };

    esp_http_client_handle_t client = esp_http_client_init(&config);
    esp_http_client_set_header(client, "Content-Type", "image/jpeg");

    esp_err_t err = esp_http_client_open(client, image_len);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Falha ao abrir cliente HTTP: %s", esp_err_to_name(err));
        return;
    }

    int written = esp_http_client_write(client, (const char *)image_data, image_len);
    if (written != image_len) {
        ESP_LOGE(TAG, "Erro escrevendo dados HTTP");
    } else {
        ESP_LOGI(TAG, "Imagem enviada com sucesso");
    }

    esp_http_client_close(client);
    esp_http_client_cleanup(client);
}

// Inicialização da câmera
esp_err_t init_camera() {
    esp_err_t err = esp_camera_init(&camera_config);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Falha ao inicializar câmera");
        return err;
    }
    return ESP_OK;
}

void app_main(void) {
    // Inicializa NVS
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);

    // Inicializa Wi-Fi
    wifi_init();

    // Inicializa câmera
    if (init_camera() != ESP_OK) {
        ESP_LOGE(TAG, "Falha crítica na inicialização da câmera");
        return;
    }

    while (true) {
        ESP_LOGI(TAG, "Capturando imagem...");

        camera_fb_t *fb = esp_camera_fb_get();
        if (!fb) {
            ESP_LOGE(TAG, "Falha ao obter frame da câmera");
            continue;
        }

        ESP_LOGI(TAG, "Imagem capturada! Tamanho: %zu bytes", fb->len);

        // Envia imagem via HTTP POST
        send_http_post(fb->buf, fb->len);

        // Libera buffer
        esp_camera_fb_return(fb);

        vTaskDelay(pdMS_TO_TICKS(5000)); // A cada 5 segundos
    }
}