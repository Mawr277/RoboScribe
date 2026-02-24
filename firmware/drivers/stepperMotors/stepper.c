/**
 * @file stepper.c
 * @author Maciej Nowicki (maciejnowicki1234@gmail.com)
 * @brief 
 * @version 0.1
 * @date 2026-02-24
 * 
 * @copyright Copyright (c) 2026
 * 
 */
#include "stepper.h"

void steppersInit(void){
    gpio_config_t motorDirConf = {
        .intr_type = GPIO_INTR_DISABLE,
        .mode = GPIO_MODE_OUTPUT,
        .pin_bit_mask = MOTOR_DIR_PINS,
        .pull_down_en = 0,
        .pull_up_en = 0,
    };
    gpio_config(&motorDirConf);

    ledc_timer_config_t ledcTimer = {
        .speed_mode       = LEDC_MODE,
        .timer_num        = LEDC_TIMER,
        .duty_resolution  = LEDC_DUTY_RES,
        .freq_hz          = LEDC_FREQ_HZ,
        .clk_cfg          = LEDC_AUTO_CLK
    };
    ledc_timer_config(&ledcTimer);

    ledc_channel_config_t ledcChannelRight = {
        .speed_mode     = LEDC_MODE,
        .channel        = LEDC_CHANNEL_0,
        .timer_sel      = LEDC_TIMER,
        .intr_type      = LEDC_INTR_DISABLE,
        .gpio_num       = MOTOR_RIGHT_STEP,
        .duty           = 0,
        .hpoint         = 0
    };
    ledc_channel_config(&ledcChannelRight);

    ledc_channel_config_t ledcChannelLeft = {
        .speed_mode     = LEDC_MODE,
        .channel        = LEDC_CHANNEL_1,
        .timer_sel      = LEDC_TIMER,
        .intr_type      = LEDC_INTR_DISABLE,
        .gpio_num       = MOTOR_LEFT_STEP,
        .duty           = 0,
        .hpoint         = 0
    };
    ledc_channel_config(&ledcChannelLeft);
}

void start_motor(ledc_channel_t ledc_channel, uint32_t speed_hz, bool direction){
    if(ledc_channel == MOTOR_CHANNEL_LEFT){
        gpio_set_level(MOTOR_LEFT_DIR, direction);
    }else{
        gpio_set_level(MOTOR_RIGHT_DIR, direction);
    }
    ledc_set_freq(LEDC_MODE, LEDC_TIMER, speed_hz);
    ledc_set_duty(LEDC_MODE, ledc_channel, LEDC_50_PERCENT);
    ledc_update_duty(LEDC_MODE, ledc_channel);
}

void stop_motor(ledc_channel_t ledc_channel)
{
    ledc_set_duty(LEDC_MODE, ledc_channel, 0);
    ledc_update_duty(LEDC_MODE, ledc_channel);
}