/**
 * This example takes a picture every 5s and print its size on serial monitor.
 */

#include "stream_server.h"

// support IDF 5.x
#ifndef portTICK_RATE_MS
#define portTICK_RATE_MS portTICK_PERIOD_MS
#endif
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


static const char *TAG = "STREAM_SERVER";
#define PART_BOUNDARY "123456789000000000000987654321"
static const char* _STREAM_CONTENT_TYPE = "multipart/x-mixed-replace;boundary=" PART_BOUNDARY;
static const char* _STREAM_BOUNDARY = "\r\n--" PART_BOUNDARY "\r\n";
static const char* _STREAM_PART = "Content-Type: image/jpeg\r\nContent-Length: %u\r\n\r\n";
static httpd_handle_t stream_httpd  = NULL;
static httpd_handle_t picture_httpd = NULL;

static camera_config_t camera_config = {
  .pin_pwdn     = CAM_PIN_PWDN,
  .pin_reset    = CAM_PIN_RESET,
  .pin_xclk     = CAM_PIN_XCLK,
  .pin_sscb_sda = CAM_PIN_SIOD,
  .pin_sscb_scl = CAM_PIN_SIOC,
  .pin_d7       = CAM_PIN_D7,
  .pin_d6       = CAM_PIN_D6,
  .pin_d5       = CAM_PIN_D5,
  .pin_d4       = CAM_PIN_D4,
  .pin_d3       = CAM_PIN_D3,
  .pin_d2       = CAM_PIN_D2,
  .pin_d1       = CAM_PIN_D1,
  .pin_d0       = CAM_PIN_D0,
  .pin_vsync    = CAM_PIN_VSYNC,
  .pin_href     = CAM_PIN_HREF,
  .pin_pclk     = CAM_PIN_PCLK,
  .xclk_freq_hz = 15000000,
  .ledc_timer   = LEDC_TIMER_0,
  .ledc_channel = LEDC_CHANNEL_0,
//   .pixel_format = PIXFORMAT_JPEG,
  .pixel_format = PIXFORMAT_RGB565, // gives better quality images
// For ESP32, do not use sizes above QVGA when not JPEG. The performance of the ESP32-S series has improved a lot, but JPEG mode always gives better frame rates.
//   .frame_size   = FRAMESIZE_QVGA,
  .frame_size   = FRAMESIZE_96X96,
  .jpeg_quality = 20,//0-63, for OV series camera sensors, lower number means higher quality
                    // NOTE: frame_size and jpeg_quality are related
                    //       if FRAMESIZE_VGA, jpeg_quality=10
                    //       if FRAMESIZE_HQVGA, jpeg_quality=15
                    //       if FRAMESIZE_QQVGA, jpeg_quality=20
                    //       if FRAMESIZE_96X96, jpeg_quality=60 (for this example is too low)
                    // otherwise the camera will not work, and the error is not obvious
  .fb_count = 1, //When jpeg mode is used, if fb_count more than one, the driver will work in continuous mode.
  .fb_location  = CAMERA_FB_IN_DRAM, 
  .grab_mode = CAMERA_GRAB_LATEST,
};

typedef struct {
    httpd_req_t *req;
    size_t len;
} jpg_chunking_t;

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

    sensor_t * s = esp_camera_sensor_get();
    if ( s != NULL )
    {
        s->set_vflip(s, 1); //flip it back
//         s->set_special_effect(s, 2); // black and white
    }
    return ESP_OK;
}

esp_err_t jpg_stream_httpd_handler(httpd_req_t *req){
    camera_fb_t * fb = NULL;
    esp_err_t res = ESP_OK;
    size_t _jpg_buf_len;
    uint8_t * _jpg_buf;
    char * part_buf[64];

    static int64_t last_frame = 0;
    if(!last_frame) {
        last_frame = esp_timer_get_time();
    }

    res = httpd_resp_set_type(req, _STREAM_CONTENT_TYPE);
    if(res != ESP_OK){
        ESP_LOGE(TAG, "Error setting resp type");
        return res;
    }

    while(true){
        fb = esp_camera_fb_get();
        if (!fb) {
            ESP_LOGE(TAG, "Camera capture failed");
            res = ESP_FAIL;
            break;
        }
        if(fb->format != PIXFORMAT_JPEG){
            bool jpeg_converted = frame2jpg(fb, 80, &_jpg_buf, &_jpg_buf_len);
            if(!jpeg_converted){
                ESP_LOGE(TAG, "JPEG compression failed");
                esp_camera_fb_return(fb);
                res = ESP_FAIL;
            }
        } else {
            _jpg_buf_len = fb->len;
            _jpg_buf = fb->buf;
        }

        if(res == ESP_OK){
            res = httpd_resp_send_chunk(req, _STREAM_BOUNDARY, strlen(_STREAM_BOUNDARY));
        }
        if(res == ESP_OK){
            size_t hlen = snprintf((char *)part_buf, 64, _STREAM_PART, _jpg_buf_len);
            res = httpd_resp_send_chunk(req, (const char *)part_buf, hlen);
        }
        if(res == ESP_OK){
            res = httpd_resp_send_chunk(req, (const char *)_jpg_buf, _jpg_buf_len);
        }
        if(fb->format != PIXFORMAT_JPEG){
            free(_jpg_buf);
        }
        esp_camera_fb_return(fb);
        if(res != ESP_OK){
            break;
        }
        int64_t fr_end = esp_timer_get_time();
        int64_t frame_time = fr_end - last_frame;
        last_frame = fr_end;
        frame_time /= 1000;
        ESP_LOGI(TAG, "MJPG: %luKB %lums (%.1ffps)",
            (uint32_t)(_jpg_buf_len/1024),
            (uint32_t)frame_time, 1000.0 / (uint32_t)frame_time);

        // add 500ms delay to reduce the frame rate
        vTaskDelay(500 / portTICK_PERIOD_MS);
    }

    last_frame = 0;
    return res;
}

// JPEG encoder
static size_t jpg_encode_stream(void * arg, size_t index, const void* data, size_t len){
    jpg_chunking_t *j = (jpg_chunking_t *)arg;
    if(!index){
        j->len = 0;
    }
    if(httpd_resp_send_chunk(j->req, (const char *)data, len) != ESP_OK){
        return 0;
    }
    j->len += len;
    return len;
}

// RGB565 handler
esp_err_t rgb_httpd_handler(httpd_req_t *req){
    camera_fb_t * fb = NULL;
    esp_err_t res = ESP_OK;
    size_t fb_len = 0;
    int64_t fr_start = esp_timer_get_time();

    fb = esp_camera_fb_get();
    if (!fb) {
        ESP_LOGE(TAG, "Camera capture failed");
        httpd_resp_send_500(req);
        return ESP_FAIL;
    }

    res = httpd_resp_set_type(req, "application/octet-stream");
    if(res == ESP_OK){
        res = httpd_resp_set_hdr(req, "Content-Disposition", "inline; filename=capture.jpg");
    }

    fb_len = fb->len;
    res = httpd_resp_send(req, (const char *)fb->buf, fb->len);

    esp_camera_fb_return(fb);
    int64_t fr_end = esp_timer_get_time();
    ESP_LOGI(TAG, "RAW: %luKB %lums", (uint32_t)(fb_len/1024), (uint32_t)((fr_end - fr_start)/1000));
    return res;

}

// JPEG HTTP Capture
esp_err_t jpg_httpd_handler(httpd_req_t *req){
    camera_fb_t * fb = NULL;
    esp_err_t res = ESP_OK;
    size_t fb_len = 0;
    int64_t fr_start = esp_timer_get_time();

    fb = esp_camera_fb_get();
    if (!fb) {
        ESP_LOGE(TAG, "Camera capture failed");
        httpd_resp_send_500(req);
        return ESP_FAIL;
    }

    res = httpd_resp_set_type(req, "image/jpeg");
    if(res == ESP_OK){
        res = httpd_resp_set_hdr(req, "Content-Disposition", "inline; filename=capture.jpg");
    }

    if(res == ESP_OK){
        if(fb->format == PIXFORMAT_JPEG){
            fb_len = fb->len;
            res = httpd_resp_send(req, (const char *)fb->buf, fb->len);
        } else {
            jpg_chunking_t jchunk = {req, 0};
            res = frame2jpg_cb(fb, 80, jpg_encode_stream, &jchunk)?ESP_OK:ESP_FAIL;
            httpd_resp_send_chunk(req, NULL, 0);
            fb_len = jchunk.len;
        }
    }
    esp_camera_fb_return(fb);
    int64_t fr_end = esp_timer_get_time();
    ESP_LOGI(TAG, "JPG: %luKB %lums", (uint32_t)(fb_len/1024), (uint32_t)((fr_end - fr_start)/1000));
    return res;
}

httpd_uri_t uri_get_picture = {
    .uri = "/picture",
    .method = HTTP_GET,
    .handler = rgb_httpd_handler,
//     .handler = jpg_httpd_handler,
    .user_ctx = NULL
};

httpd_uri_t uri_stream = {
    .uri = "/",
    .method = HTTP_GET,
    .handler = jpg_stream_httpd_handler,
    .user_ctx = NULL
};


httpd_handle_t setup_stream_server(void)
{
    httpd_config_t config = HTTPD_DEFAULT_CONFIG();

    ESP_LOGI(TAG, "Starting stream server on port: '%d'", config.server_port);
    if (httpd_start(&stream_httpd , &config) == ESP_OK)
    {
        httpd_register_uri_handler(stream_httpd , &uri_stream);
    }
    return stream_httpd;
}

httpd_handle_t setup_picture_server(void)
{
    httpd_config_t config = HTTPD_DEFAULT_CONFIG();

    config.server_port += 1;
    config.ctrl_port += 1;
    ESP_LOGI(TAG, "Starting picture server on port: '%d'", config.server_port);
    if (httpd_start(&picture_httpd , &config) == ESP_OK)
    {
        httpd_register_uri_handler(picture_httpd , &uri_get_picture);
    }
    return picture_httpd;
}

StreamServer::StreamServer(){
    ESP_ERROR_CHECK(init_camera());
}

StreamServer::~StreamServer(){
    httpd_stop(stream_httpd);
    esp_camera_deinit();
}

void StreamServer::Start(){
//     httpd_handle_t stream_httpd = setup_stream_server();
//     if(stream_httpd == NULL){
//         ESP_LOGE(TAG, "Failed to start stream server");
//     }
    httpd_handle_t picture_httpd = setup_picture_server();
    if(picture_httpd == NULL){
        ESP_LOGE(TAG, "Failed to start picture server");
    }
}

void StreamServer::Stop(){
    httpd_stop(stream_httpd);
}
