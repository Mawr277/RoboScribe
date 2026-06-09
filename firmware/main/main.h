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
#include "driver/i2c_master.h"
#include "esp_intr_types.h"
#include "esp_timer.h"
#include "esp_vfs.h"
#include "esp_err.h"
#include "esp_log.h"
#include "nvs_flash.h"
#include "esp_event.h"
#include "sdkconfig.h"

#include "protocol_examples_common.h"
#include "MPU6050.h"
#include "file_server.h"
#include "robot_data.h"
#include "SDcard.h"
#include "servo.h"

#define CORE0   0
#define CORE1   1

#define LEDC_TIMER_0             LEDC_TIMER_0
#define SERVO_CHANNEL_0          LEDC_CHANNEL_0
#define LEDC_TIMER_1             LEDC_TIMER_1
#define SERVO_CHANNEL_1          LEDC_CHANNEL_1

#define BUTTON_GPIO        21
#define GPIO_INPUT_PIN_SEL 1ULL<<BUTTON_GPIO
#define ESP_INTR_FLAG_DEFAULT 0

#define I2C_SDA                     8
#define I2C_SCL                     18
#define I2C_MASTER_NUM              I2C_NUM_0

const float valTOms2 = 9.81/16384;

/// @brief Command read from .gcode file line
typedef struct {
    uint8_t num;    /// Command number - 0, 1 ...
    int16_t a, b;     /// Axes/angles parameters -- x, y lub v, t
    int16_t tool;      /// Flag - which axes were given
}command;

static const char *TAG = "roboScribe";

static void gcodeParser(FILE *program);
static void executeCmd(command *cmd);