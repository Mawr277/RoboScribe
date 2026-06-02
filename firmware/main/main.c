/**
 * @file main.c
 * @author Maciej Nowicki (maciejnowicki1234@gmail.com)
 * @author Maciej Nowicki
 * @brief 
 * @version 0.1
 * @date 2026-01-09
 * 
 * @copyright Copyright (c) 2026
 */
#include "main.h"

static bool started = false;

servomotor servo_tool= {
    .pin = 14,
    .freq = 50,
    .channel = LEDC_CHANNEL_0,
    .duty_0 = 170.0f,
    .duty_180 = 1050.0f
};

servomotor servo_base = {
    .pin = 16,
    .freq = 50,
    .channel = LEDC_CHANNEL_0,
    .duty_0 = 190.0f,
    .duty_180 = 950.0f
};

servomotor servo_arm = {
    .pin = 15,
    .freq = 50,
    .channel = LEDC_CHANNEL_1,
    .duty_0 = 190.0f,
    .duty_180 = 1000.0f
};


void gcodeParser(FILE *program){
    char line[MAX_CHAR_SIZE];

    while(fgets(line, sizeof(line), program)){
        command cmd = {0};
        char *ptr = line;

        cmd.num = (uint8_t)strtol(ptr+1, &ptr, 10);
        while(*ptr != '\0' && *ptr != '\n' && *ptr != '\r'){
            if(*ptr == ' '){
                ptr++;
                continue;
            }
            if(*ptr == 'T' || *ptr == 'X'){
                cmd.a = (uint8_t)strtol(ptr+1, &ptr, 10);
            }else if(*ptr == 'V' || *ptr == 'Y'){
                cmd.b = (uint8_t)strtol(ptr+1, &ptr, 10);
            }else if(*ptr == 'Z'){
                cmd.tool = (uint8_t)strtol(ptr+1, &ptr, 10);
            }else{
                ptr++;
            }
        }

        executeCmd(&cmd);
    }
}

void executeCmd(command *cmd){
    switch(cmd->num){
        case 0:
            ESP_LOGI(TAG, "G00 a=%u b=%u", cmd->a, cmd->b);
            vTaskDelay(pdMS_TO_TICKS(2000));
            break;
        case 1:
            ESP_LOGI(TAG, "G01 tool=%u", cmd->tool);
            if (cmd->tool == 0){
                ESP_LOGI(TAG, "TOOL DOWN");
                servo_set_position(servo_tool, 130);
            }else{
                ESP_LOGI(TAG, "TOOL UP");
                servo_set_position(servo_tool, 50); // UP
            }
            vTaskDelay(pdMS_TO_TICKS(200));
            break;
        case 3:
            ESP_LOGI(TAG, "G03 a=%u b=%u", cmd->a, cmd->b);
            servo_set_position(servo_base, cmd->a);
            servo_set_position(servo_arm, 180-cmd->b);
            vTaskDelay(pdMS_TO_TICKS(200));
            break;
        default:
            ESP_LOGE(TAG, "Command not found");
            break;
    }

    started = false;
} 

void app_main(){
    esp_err_t ret;
    sdmmc_card_t *card = NULL;
    ret = card_init(&card);
    if (ret != ESP_OK) {
        return;
    }

    servo_ledc_init(servo_base);
    servo_ledc_init(servo_arm);
    servo_ledc_init(servo_tool);

    gpio_config_t io_conf = {};
    io_conf.intr_type = GPIO_INTR_DISABLE;
    io_conf.mode = GPIO_MODE_INPUT;
    io_conf.pin_bit_mask = GPIO_INPUT_PIN_SEL;
    io_conf.pull_down_en = GPIO_PULLDOWN_DISABLE;
    io_conf.pull_up_en = GPIO_PULLUP_ENABLE;
    gpio_config(&io_conf);

    while(1){
        while (!started) {
            if (gpio_get_level(BUTTON) == 0) {
                vTaskDelay(pdMS_TO_TICKS(50));
                if (gpio_get_level(BUTTON) == 0) {
                    started = true;
                }
            }
            vTaskDelay(pdMS_TO_TICKS(10));
        }

        const char *filename = "/L_output.gcode";
        char program[256];
        snprintf(program, sizeof(program), "%s%s", MOUNT_POINT, filename);
        
        if (started) {
            ESP_LOGI(TAG, "%s", program);
            ret = sd_read_file(program, &gcodeParser);
            if (ret != ESP_OK) {
                return;
            }
        }
    }
}
