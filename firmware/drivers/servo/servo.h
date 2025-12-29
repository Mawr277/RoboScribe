/*
    Headere file for TowerPro MG90S i SG90 servo control 
*/

#include <stdio.h>
#include "freertos\FreeRTOS.h"
#include "driver/ledc.h"
#include "esp_err.h"

#define SERVO_LEDC_MODE           LEDC_LOW_SPEED_MODE
#define SERVO_LEDC_DUTY_RES       LEDC_TIMER_13_BIT // Set duty resolution
#define SERVO_DUTY_0              170.0f  // Servo duty for position 0
#define SERVO_DUTY_180            1050.0f // Servo duty for position 180
#define SERVO_LEDC_FREQUENCY      50  // Frequency in Hertz

void servo_ledc_init(ledc_timer_t ledc_timer, 
                    ledc_channel_t ledc_channel,
                    int ledc_output_io);
esp_err_t servo_set_position(ledc_channel_t servo_channel, 
                    float angle);