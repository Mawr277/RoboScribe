#ifndef ROBOT_DATA_H
#define ROBOT_DATA_H

#include <stdint.h>
#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"

typedef enum {
    STOP,
    START,
    PAUSE,
    UNPAUSE
}robot_state;

typedef struct {
    uint16_t x_accel;
    uint16_t y_accel;
    uint16_t z_accel;
} accel_vector;

typedef struct {
    uint8_t base_angle;
    uint8_t arm_angle;
    uint8_t tool_angle;
    uint8_t x_coord;
    uint8_t y_coord;
} mov_controls;

extern robot_state current_state;
extern accel_vector imu_readings;

extern QueueHandle_t manual_mov_queue;

extern SemaphoreHandle_t imu_readings_mutex; 
extern SemaphoreHandle_t sd_card_mutex;
extern SemaphoreHandle_t robot_state_mutex;
extern char filename[];


#endif