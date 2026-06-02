#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "servo.h"

servomotor servo_tool= {
    .pin = 14,
    .freq = 50,
    .channel = LEDC_CHANNEL_2,
    .duty_0 = 170.0f,
    .duty_180 = 1050.0f
};

servomotor servo_base = {
    .pin = 16,
    .freq = 50,
    .channel = LEDC_CHANNEL_0,
    .duty_0 = 190.0f,
    .duty_180 = 950.0f
};

servomotor servo_arm = {
    .pin = 15,
    .freq = 50,
    .channel = LEDC_CHANNEL_1,
    .duty_0 = 190.0f,
    .duty_180 = 1000.0f
};


void app_main(void)
{
    servo_ledc_init(servo_base);
    servo_ledc_init(servo_arm);
    servo_ledc_init(servo_tool);
    while(1){
        servo_set_position(servo_arm, 0);
        vTaskDelay(pdMS_TO_TICKS(3000));
        servo_set_position(servo_arm, 45);
        // servo_set_position(servo_arm, 0);
        vTaskDelay(pdMS_TO_TICKS(3000));
        servo_set_position(servo_arm, 180);
        // servo_set_position(servo_arm, 180);
        vTaskDelay(pdMS_TO_TICKS(3000));

        servo_set_position(servo_tool, 130.0f);
        for(float a = 60.0f; a <= 120; a += 30){
            servo_set_position(servo_arm, a);
            servo_set_position(servo_base, a);
            vTaskDelay(pdMS_TO_TICKS(500));
        }
        servo_set_position(servo_tool, 50.0f);
        vTaskDelay(pdMS_TO_TICKS(200));
        servo_set_position(servo_base, 60.0f);
        vTaskDelay(pdMS_TO_TICKS(2000));
    }
}
