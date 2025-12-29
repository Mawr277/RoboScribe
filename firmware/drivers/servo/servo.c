#include "servo.h"

/**
 * @brief Set the LEDC peripheral configuration
 */
void servo_ledc_init(ledc_timer_t ledc_timer_type, 
                          ledc_channel_t ledc_channel_num,
                          int ledc_output_io){
    // Prepare and then apply the LEDC PWM timer configuration
    ledc_timer_config_t ledc_timer = {
        .speed_mode       = SERVO_LEDC_MODE,
        .duty_resolution  = SERVO_LEDC_DUTY_RES,
        .timer_num        = ledc_timer_type,
        .freq_hz          = SERVO_LEDC_FREQUENCY,
        .clk_cfg          = LEDC_AUTO_CLK
    };
    ESP_ERROR_CHECK(ledc_timer_config(&ledc_timer));

    // Prepare and then apply the LEDC PWM channel configuration
    ledc_channel_config_t ledc_channel = {
        .speed_mode     = SERVO_LEDC_MODE,
        .channel        = ledc_channel_num,
        .timer_sel      = ledc_timer_type,
        .intr_type      = LEDC_INTR_DISABLE,
        .gpio_num       = ledc_output_io,
        .duty           = 0, // Set duty to position 0
        .hpoint         = 0
    };
    ESP_ERROR_CHECK(ledc_channel_config(&ledc_channel));
}

/**
 * @brief Set servo of servo_channel position for desired angle
 */
esp_err_t servo_set_position(ledc_channel_t servo_channel, float angle){
    const float angleCoeff = (SERVO_DUTY_180 - SERVO_DUTY_0)/180.0f;
    uint32_t duty = (uint32_t)(SERVO_DUTY_0 + angle*angleCoeff);
    printf("Duty: %ld, Angle: %f\n", duty, angle);
    ESP_ERROR_CHECK(ledc_set_duty(SERVO_LEDC_MODE, servo_channel, duty));
    return ledc_update_duty(SERVO_LEDC_MODE, servo_channel);
}