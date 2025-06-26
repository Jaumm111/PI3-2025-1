#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "driver/ledc.h"
#include <stdio.h>

#define PIN_EN      GPIO_NUM_32
#define PIN_EST     GPIO_NUM_16
#define PIN_STEP_0  GPIO_NUM_15 //STEP cancela
#define PIN_DIR_0   GPIO_NUM_14 //DIR cancela
#define PIN_STEP_1  GPIO_NUM_12 //STEP porta 1
#define PIN_DIR_1   GPIO_NUM_13 //DIR porta 1
#define PIN_SLEEP   GPIO_NUM_27  // connect SLEEP to GPIO27 or tie to 3.3V if unused

void abre_porta(void){
    int direction = 1;

    // Run in current direction
    gpio_set_level(PIN_DIR_1, direction);  // Set direction
    ledc_set_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_1, 512); // Resume PWM
    ledc_update_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_1);

    vTaskDelay(pdMS_TO_TICKS(400));  // Run for 1 second

    // Stop PWM (motor idle)
    ledc_set_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_1, 0);
    ledc_update_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_1);
}
void fecha_porta(void){
    int direction = 0;

    // Run in current direction
    gpio_set_level(PIN_DIR_1, direction);  // Set direction
    ledc_set_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_1, 512); // Resume PWM
    ledc_update_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_1);

    vTaskDelay(pdMS_TO_TICKS(400));  // Run for 1 second

    // Stop PWM (motor idle)
    ledc_set_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_1, 0);
    ledc_update_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_1);
}
void libera_cancela(void){
    int direction = 1;

    // Run in current direction
    gpio_set_level(PIN_DIR_0, direction);  // Set direction
    ledc_set_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_0, 512); // Resume PWM
    ledc_update_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_0);

    vTaskDelay(pdMS_TO_TICKS(900));  // Run for 1 second

    // Stop PWM (motor idle)
    ledc_set_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_0, 0);
    ledc_update_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_0);

    vTaskDelay(pdMS_TO_TICKS(1500));  // Wait 1 second before changing direction

    // Toggle direction
    direction = !direction;

    // Run in current direction
    gpio_set_level(PIN_DIR_0, direction);  // Set direction
    ledc_set_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_0, 512); // Resume PWM
    ledc_update_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_0);

    vTaskDelay(pdMS_TO_TICKS(900));  // Run for 1 second

    // Stop PWM (motor idle)
    ledc_set_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_0, 0);
    ledc_update_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_0);
}

void app_main(void)
{
    // Configure pins
    gpio_config_t io_conf = {
        .pin_bit_mask = (1ULL << PIN_EN) | (1ULL << PIN_EST) | (1ULL << PIN_DIR_0)| (1ULL << PIN_DIR_1),
        .mode = GPIO_MODE_OUTPUT,
        .pull_up_en = GPIO_PULLUP_DISABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_DISABLE,
    };
    gpio_config(&io_conf);

    gpio_set_level(PIN_EST, 1);
    // STEP pin config separately for PWM
    gpio_set_direction(PIN_STEP_0, GPIO_MODE_OUTPUT);
    gpio_set_direction(PIN_STEP_1, GPIO_MODE_OUTPUT);

    // Reset pulse: RST low then high
    /*gpio_set_level(PIN_RST, 0);
    vTaskDelay(pdMS_TO_TICKS(10));
    gpio_set_level(PIN_RST, 1);
    */
    // Enable the driver (active LOW)
    gpio_set_level(PIN_EN, 0);

    // Wake up driver (active HIGH)

    // Setup LEDC PWM for STEP pin at 300Hz, 50% duty
    ledc_timer_config_t ledc_timer = {
        .speed_mode = LEDC_LOW_SPEED_MODE,
        .timer_num = LEDC_TIMER_0,
        .duty_resolution = LEDC_TIMER_10_BIT,  // 10-bit resolution (0-1023)
        .freq_hz = 1200,
        .clk_cfg = LEDC_AUTO_CLK,
    };
    ledc_timer_config(&ledc_timer);

    ledc_channel_config_t ledc_channel_0 = {
        .gpio_num = PIN_STEP_0,
        .speed_mode = LEDC_LOW_SPEED_MODE,
        .channel = LEDC_CHANNEL_0,
        .timer_sel = LEDC_TIMER_0,
        .duty = 512,  // 50%
        .hpoint = 0,
    };
    ledc_channel_config_t ledc_channel_1 = {
        .gpio_num = PIN_STEP_1,
        .speed_mode = LEDC_LOW_SPEED_MODE,
        .channel = LEDC_CHANNEL_1,
        .timer_sel = LEDC_TIMER_0,
        .duty = 512,  // 50%
        .hpoint = 0,
    };
    ledc_channel_config(&ledc_channel_0);
    ledc_channel_config(&ledc_channel_1);

    ledc_set_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_0, 0);
    ledc_update_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_0);

    ledc_set_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_1, 0);
    ledc_update_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_1);

    while (1) {
        /*// Run in current direction
        gpio_set_level(PIN_DIR, direction);  // Set direction
        ledc_set_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_0, 512); // Resume PWM
        ledc_update_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_0);

        vTaskDelay(pdMS_TO_TICKS(900));  // Run for 1 second

        // Stop PWM (motor idle)
        ledc_set_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_0, 0);
        ledc_update_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_0);

        vTaskDelay(pdMS_TO_TICKS(1500));  // Wait 1 second before changing direction

        // Toggle direction
        direction = !direction;*/

        abre_porta();
        
        vTaskDelay(pdMS_TO_TICKS(100));
        gpio_set_level(PIN_EST, 0);
        libera_cancela();

        vTaskDelay(pdMS_TO_TICKS(1500));

        fecha_porta();
        gpio_set_level(PIN_EST, 1);
        vTaskDelay(pdMS_TO_TICKS(3000));

    }
}
