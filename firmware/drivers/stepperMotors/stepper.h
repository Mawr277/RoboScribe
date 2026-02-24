/**
 * @file stepper.h
 * @author Maciej Nowicki (maciejnowicki1234@gmail.com)
 * @brief 
 * @version 0.1
 * @date 2026-02-24
 * 
 * @copyright Copyright (c) 2026
 * 
 */
#include "driver/gpio.h"
#include "driver/ledc.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#define MOTOR_LEFT_STEP     1
#define MOTOR_LEFT_DIR      2
#define MOTOR_RIGHT_STEP    5
#define MOTOR_RIGHT_DIR     4
#define MOTOR_DIR_PINS      ((1ULL<<MOTOR_LEFT_DIR) | (1ULL<<MOTOR_RIGHT_DIR))
#define STEPS_PER_REV       6400

#define MOTOR_CHANNEL_RIGHT LEDC_CHANNEL_0      
#define MOTOR_CHANNEL_LEFT  LEDC_CHANNEL_1      
#define LEDC_TIMER          LEDC_TIMER_0
#define LEDC_FREQ_HZ        1000
#define LEDC_MODE           LEDC_LOW_SPEED_MODE
#define LEDC_DUTY_RES       LEDC_TIMER_10_BIT
#define LEDC_50_PERCENT     512

void steppersInit(void);
void start_motor(ledc_channel_t ledc_channel, uint32_t speed_hz, bool direction);
void stop_motor(ledc_channel_t ledc_channel);