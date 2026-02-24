#include <rom/ets_sys.h> // Dla ets_delay_us
#include "stepper.h"
#include "driver/gpio.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

void app_main(void)
{
    steppersInit();

    while (1) {
        start_motor(MOTOR_CHANNEL_RIGHT, 1000, 1);
        start_motor(MOTOR_CHANNEL_LEFT, 1000, 0);
        vTaskDelay(pdMS_TO_TICKS(3000));

        stop_motor(MOTOR_CHANNEL_RIGHT);
        stop_motor(MOTOR_CHANNEL_LEFT);
        vTaskDelay(pdMS_TO_TICKS(1000));

        start_motor(MOTOR_CHANNEL_RIGHT, 1000, 0);
        start_motor(MOTOR_CHANNEL_LEFT, 1000, 1);
        vTaskDelay(pdMS_TO_TICKS(3000));

        stop_motor(MOTOR_CHANNEL_RIGHT);
        stop_motor(MOTOR_CHANNEL_LEFT);
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}
