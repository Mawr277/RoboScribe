/**
 * @file main.h
 * @author Maciej Nowicki (maciejnowicki1234@gmail.com)
 * @author Maciej Nowicki
 * @brief 
 * @version 0.1
 * @date 2026-01-09
 * 
 * @copyright Copyright (c) 2026
 * 
 */

#pragma once
#include <stdarg.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "esp_log.h"
#include "SDcard.h"
#include "servo.h"

#define LEDC_TIMER_0             LEDC_TIMER_0
#define SERVO_CHANNEL_0          LEDC_CHANNEL_0
#define LEDC_TIMER_1             LEDC_TIMER_1
#define SERVO_CHANNEL_1          LEDC_CHANNEL_1

#define BUTTON             21
#define GPIO_INPUT_PIN_SEL 1ULL<<BUTTON

/// @brief Command read from .gcode file line
typedef struct {
    uint8_t num;    /// Command number - 0, 1 ...
    int16_t a, b;     /// Axes/angles parameters -- x, y lub v, t
    int16_t tool;      /// Flag - which axes were given
}command;

static const char *TAG = "Main";

void gcodeParser(FILE *program);
void executeCmd(command *cmd);