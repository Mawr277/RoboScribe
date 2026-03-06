// #include <rom/ets_sys.h> // Dla ets_delay_us
#include "driver/gpio.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "stepper.h"

void app_main(void)
{
    initSteppers();
    uint32_t direction = 1;
    uint32_t speedHz = 3000;
    uint64_t stepsNum = 4000;

    while(1){    
        robotMove(direction, speedHz, stepsNum);
        vTaskDelay(pdMS_TO_TICKS(4000));
        robotRotate(direction, speedHz, stepsNum);
        vTaskDelay(pdMS_TO_TICKS(4000));
    }
}