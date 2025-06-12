#ifndef STREAM_SERVER_H
#define STREAM_SERVER_H

#include <esp_log.h>
#include <esp_system.h>
#include <nvs_flash.h>
#include <sys/param.h>
#include <string.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "esp_camera.h"
#include "esp_http_server.h"
#include "esp_timer.h"

#include "esp_mac.h"

class StreamServer {
    public:
        StreamServer();
        ~StreamServer();
        void Start();
        void Stop();
};

#endif // STREAM_SERVER_H
