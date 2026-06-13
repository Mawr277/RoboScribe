/**
 * @file stepperMotorEncoder.c
 * @brief Plik nagłówkowy enkodera rmt dla silników krokowych
 * @version 0.1
 * @date 2026-03-06
 * @ingroup drivers
 * 
 * @copyright Copyright (c) 2026
 * 
 */
#pragma once

#include <stdint.h>
#include "driver/rmt_encoder.h"
#include "esp_check.h"

typedef struct {
    uint32_t resolution; // Encoder resolution, in Hz
} stepper_motor_uniform_encoder_config_t;

/// @brief Dane enkodera rmt dla silnika krokowego
typedef struct {
    rmt_encoder_t base;
    rmt_encoder_handle_t copy_encoder;
    uint32_t resolution;
} rmt_stepper_uniform_encoder_t;

/**
 * @brief Create RMT encoder for encoding step motor uniform phase into RMT symbols
 *
 * @param[in] config Encoder configuration
 * @param[out] ret_encoder Returned encoder handle
 * @return
 *      - ESP_ERR_INVALID_ARG for any invalid arguments
 *      - ESP_ERR_NO_MEM out of memory when creating step motor encoder
 *      - ESP_OK if creating encoder successfully
 */
esp_err_t rmt_new_stepper_motor_uniform_encoder(const stepper_motor_uniform_encoder_config_t *config, rmt_encoder_handle_t *ret_encoder);