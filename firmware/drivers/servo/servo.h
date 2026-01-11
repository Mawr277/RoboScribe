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

/** 
 * @def SERVO_DUTY_0
 * @brief Współczynnik wypełnienia dla pozycji 0 stopni
 * ustawiana po badaniu empirycznym, by można było sterować serwem
 * przy pomocy esp32 bez zastosowania przetwornika poziomów logicznych.
 * \n Wykorzystywana przy przeliczaniu kąta na współczynnik wypełnienia (duty cycle).
 */
/** 
 * @def SERVO_DUTY_180
 * @brief Współczynnik wypełnienia dla pozycji 180 stopni
 * ustawiana po badaniu empirycznym, by można było sterować serwem
 * przy pomocy esp32 bez zastosowania przetwornika poziomów logicznych.
 * \n Wykorzystywana przy przeliczaniu kąta na współczynnik wypełnienia (duty cycle).
 */
#define SERVO_LEDC_MODE           LEDC_LOW_SPEED_MODE
#define SERVO_LEDC_DUTY_RES       LEDC_TIMER_13_BIT
#define SERVO_DUTY_0              170.0f 
#define SERVO_DUTY_180            1050.0f
#define SERVO_LEDC_FREQUENCY      50

/**
 * @brief Inicjalizacja i konfiguracja serwa
 * 
 * @param ledc_timer typ stosowanego timeru dla serwa
 * @param ledc_channel numer kanału dla serwa
 * @param ledc_output_io numper pinu serwa
 */
void servo_ledc_init(ledc_timer_t ledc_timer, 
                    ledc_channel_t ledc_channel,
                    int ledc_output_io);

/**
 * @brief Ustawienie pozycji serwa
 * 
 * @param servo_channel numer kanału serwa
 * @param angle kąt
 */
void servo_set_position(ledc_channel_t servo_channel, 
                    float angle);