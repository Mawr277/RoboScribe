#include <stdio.h>
#include "freertos\FreeRTOS.h"
#include "driver/ledc.h"
#include "esp_err.h"

#include "servo.h"

#define LEDC_TIMER              LEDC_TIMER_0
#define SERVO_CHANNEL            LEDC_CHANNEL_0

void app_main(void)
{
    const int servoPin = 10;

    servo_ledc_init(LEDC_TIMER, SERVO_CHANNEL, servoPin);
    while(1){
        for(float a = 0.0f; a < 181; a += 10){
            ESP_ERROR_CHECK(servo_set_position(SERVO_CHANNEL, a));
            vTaskDelay(pdMS_TO_TICKS(200));
        }
        vTaskDelay(pdMS_TO_TICKS(5000));
    }
}
