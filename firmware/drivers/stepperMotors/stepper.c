/**
 * @file stepper.c
 * @author Maciej Nowicki (maciejnowicki1234@gmail.com)
 * @brief Stepper motor control driver using recommended rmt implementation
 * @version 0.1
 * @date 2026-03=06
 * 
 * @copyright Copyright (c) 2026
 * 
 */
#include <stdbool.h>
#include "stepper.h"
#include "stepperMotorEncoder.h"

static rmt_channel_handle_t motorChannels[2] = {NULL};
static rmt_encoder_handle_t uniformMotorEncoder = NULL;
static rmt_sync_manager_handle_t motorSyncManager = NULL;
static rmt_transmit_config_t txConfig;

void initSteppers(void){
    gpio_config_t dirGpioConfig = {
        .mode = GPIO_MODE_OUTPUT,
        .intr_type = GPIO_INTR_DISABLE,
        .pin_bit_mask = 1ULL << MOTOR_LEFT_DIR | 1ULL << MOTOR_RIGHT_DIR,
    };
    ESP_ERROR_CHECK(gpio_config(&dirGpioConfig));

    uint8_t motorGpio[2] = {MOTOR_LEFT_STEP, MOTOR_RIGHT_STEP};

    // RMT channels initialization
    for (int i = 0; i < 2; i++) {
        rmt_tx_channel_config_t txChanLeftConfig = {
            .clk_src = RMT_CLK_SRC_DEFAULT, // select clock source
            .gpio_num = motorGpio[i],
            .mem_block_symbols = 64,
            .resolution_hz = STEP_MOTOR_RESOLUTION_HZ,
            .trans_queue_depth = 10, // set the number of transactions that can be pending in the background
        };
        ESP_ERROR_CHECK(rmt_new_tx_channel(&txChanLeftConfig, &motorChannels[i]));
    }

    // RMT encoder
    stepper_motor_uniform_encoder_config_t uniform_encoder_config = {
        .resolution = STEP_MOTOR_RESOLUTION_HZ,
    };
    ESP_ERROR_CHECK(rmt_new_stepper_motor_uniform_encoder(&uniform_encoder_config, &uniformMotorEncoder));

    for (int i = 0; i < 2; i++) {
        ESP_ERROR_CHECK(rmt_enable(motorChannels[i]));
    }

    // RMT sync manager
    rmt_sync_manager_config_t syncManagerConfig = {
        .tx_channel_array = motorChannels,
        .array_size = sizeof(motorChannels) / sizeof(motorChannels[0]),
    };
    ESP_ERROR_CHECK(rmt_new_sync_manager(&syncManagerConfig, &motorSyncManager));

    txConfig.loop_count = 0;
}

void robotMove(uint32_t direction, uint32_t speedHz, uint64_t stepsNum){
    gpio_set_level(MOTOR_LEFT_DIR, direction);
    gpio_set_level(MOTOR_RIGHT_DIR, direction);

    txConfig.loop_count = stepsNum;
    for (int i = 0; i < 2; i++) {
        ESP_ERROR_CHECK(rmt_transmit(motorChannels[i], uniformMotorEncoder, &speedHz, sizeof(speedHz), &txConfig));
    }
    
    for (int i = 0; i < 2; i++) {
        ESP_ERROR_CHECK(rmt_tx_wait_all_done(motorChannels[i], -1));
    }
}

void robotRotate(uint32_t direction, uint32_t speedHz, uint64_t stepsNum){
    gpio_set_level(MOTOR_LEFT_DIR, direction);
    gpio_set_level(MOTOR_RIGHT_DIR, !direction);

    txConfig.loop_count = stepsNum;
    for (int i = 0; i < 2; i++) {
        ESP_ERROR_CHECK(rmt_transmit(motorChannels[i], uniformMotorEncoder, &speedHz, sizeof(speedHz), &txConfig));
    }

    for (int i = 0; i < 2; i++) {
        ESP_ERROR_CHECK(rmt_tx_wait_all_done(motorChannels[i], -1));
    }
}
