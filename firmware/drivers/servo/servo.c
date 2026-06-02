/**
 * @file servo.c
 * @author Maciej Nowicki (maciejnowicki1234@gmail.com)
 * @brief Upraszcza implementacje serwomotorów TowerPro MG90S i SG90.
 * \n W tym celu stosuje się bibliotekę ledc do generowania sygnałów PWM.
 * @version 1.00
 * @date 2026-12-08
 * @ingroup drivers
 * 
 * @copyright Copyright (c) 2026
 * 
 */
#include "servo.h"

void servo_ledc_init(servomotor servo){
    // Przygotuj i później zastosuj konfigurację timera PWM
    ledc_timer_config_t ledc_timer = {
        .speed_mode       = SERVO_LEDC_MODE,
        .duty_resolution  = SERVO_LEDC_DUTY_RES,
        .timer_num        = LEDC_TIMER_0,
        .freq_hz          = servo.freq,
        .clk_cfg          = LEDC_AUTO_CLK
    };
    ESP_ERROR_CHECK(ledc_timer_config(&ledc_timer));

    // Przygotuj i później zastosuj konfigurację kanału PWM
    ledc_channel_config_t ledc_channel = {
        .speed_mode     = SERVO_LEDC_MODE,
        .channel        = servo.channel,
        .timer_sel      = LEDC_TIMER_0,
        .intr_type      = LEDC_INTR_DISABLE,
        .gpio_num       = servo.pin,
        .duty           = 0,
        .hpoint         = 0
    };
    ESP_ERROR_CHECK(ledc_channel_config(&ledc_channel));
}

void servo_set_position(servomotor servo, float angle){
    const float angleCoeff = (servo.duty_180 - servo.duty_0)/180.0f; //< Zmiana wsp. wypełnienia odpowiadająca zmianie kąta o 1 stopień
    uint32_t duty = (uint32_t)(servo.duty_0 + angle*angleCoeff);     //< Współczynnik wypełnienia dla zadanego kąta
    ESP_ERROR_CHECK(ledc_set_duty(SERVO_LEDC_MODE, servo.channel, duty));
    ESP_ERROR_CHECK(ledc_update_duty(SERVO_LEDC_MODE, servo.channel));
}