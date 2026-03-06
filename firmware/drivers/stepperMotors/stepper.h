/**
 * @file stepper.h
 * @author Maciej Nowicki (maciejnowicki1234@gmail.com)
 * @brief Header file for stepper motor driver
 * @version 0.1
 * @date 2026-02-24
 * 
 * @copyright Copyright (c) 2026
 * 
 */

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/rmt_tx.h"
#include "driver/gpio.h"
#include "esp_err.h"

#define MOTOR_LEFT_STEP     1
#define MOTOR_LEFT_DIR      2
#define MOTOR_RIGHT_STEP    5
#define MOTOR_RIGHT_DIR     4
#define STEP_MOTOR_ENABLE_LEVEL  0 // DRV8825 is enabled on low level
#define STEP_MOTOR_SPIN_DIR_CLOCKWISE 0
#define STEP_MOTOR_SPIN_DIR_COUNTERCLOCKWISE !STEP_MOTOR_SPIN_DIR_CLOCKWISE

#define STEP_MOTOR_RESOLUTION_HZ 1000000 // 1MHz resolution

/**
 * @brief Stepper motor and rmt initialization
 * 
 */
void initSteppers(void);

/**
 * @brief Move robot forward/backward
 * 
 * @param direction move robot 0 - backwward, 1 - forward
 * @param speedHz robot speed in PWM signal frequency
 * @param stepsNum number of motor steps
 */
void robotMove(uint32_t direction, uint32_t speedHz, uint64_t stepsNum);
/**
 * @brief Rotate robot
 * 
 * @param direction move robot 1 - right, 0 - left
 * @param speedHz robot speed in PWM signal frequency
 * @param stepsNum number of motor steps
 */
void robotRotate(uint32_t direction, uint32_t speedHz, uint64_t stepsNum);