/**
 * @file servo.h
 * @author Maciej Nowicki (maciejnowicki1234@gmail.com)
 * @brief Plik nagłówkowy do obsługi serwomotorów TowerPro MG90S i SG90
 * @version 1.00
 * @date 2026-12-08
 * @ingroup drivers
 * 
 * @copyright Copyright (c) 2026
 * 
 */
#include <stdio.h>
#include "driver/ledc.h"
#include "esp_err.h"

#define SERVO_LEDC_MODE           LEDC_LOW_SPEED_MODE
#define SERVO_LEDC_DUTY_RES       LEDC_TIMER_13_BIT
#define LEDC_TIMER                LEDC_TIMER_0

/// @brief Struktura z danymi do obsługi serwa
typedef struct {
    /// Numer pinu
    uint8_t pin;
    /// Częstotliwośc sterującego sygnału PWM            
    uint16_t freq;
    /// Kanał mikrokontrolera dla serwa          
    ledc_channel_t channel; 
    /// Bitowa długość stanu wysokiego dla pozycji 0 stopni
    uint16_t duty_0;  
    /// itowa długość stanu wysokiego dla pozycji 180 stopni 
    uint16_t duty_180;
}servomotor;

/**
 * @brief Inicjalizacja i konfiguracja serwa
 * 
 * @param servo parametry serwa
 */
void servo_ledc_init(servomotor servo);

/**
 * @brief Ustawienie pozycji serwa
 * 
 * @param servo parametry serwa
 * @param angle kąt
 */
void servo_set_position(servomotor servo, 
                    float angle);