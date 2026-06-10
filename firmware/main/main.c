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

const uint8_t manual_mov_queue_len = 8;

robot_state current_state = STOP;
char filename[64];
accel_vector imu_readings = {0};
mov_controls manual_mov = {0};

TaskHandle_t gcode_task_handle = NULL;
TaskHandle_t manual_control_task_handle = NULL;
TaskHandle_t imu_task_handle = NULL;

QueueHandle_t manual_mov_queue = NULL;
SemaphoreHandle_t imu_readings_mutex = NULL; 
SemaphoreHandle_t sd_card_mutex = NULL; 
SemaphoreHandle_t robot_state_mutex = NULL;

volatile bool started = false;
static const int64_t DEBOUNCE_TIME_US = 50000;
volatile int64_t last_isr_time = 0;

servomotor servo_tool= {
    .pin = 14,
    .freq = 50,
    .channel = LEDC_CHANNEL_0,
    .duty_0 = 190.0f,
    .duty_180 = 700.0f
};

servomotor servo_base = {
    .pin = 16,
    .freq = 50,
    .channel = LEDC_CHANNEL_1,
    .duty_0 = 190.0f,
    .duty_180 = 930.0f
};

servomotor servo_arm = {
    .pin = 15,
    .freq = 50,
    .channel = LEDC_CHANNEL_2,
    .duty_0 = 190.0f,
    .duty_180 = 930.0f
};

void IRAM_ATTR start_execution_isr(){
    int64_t current_time = esp_timer_get_time();

    if ((current_time - last_isr_time) > DEBOUNCE_TIME_US) {
        last_isr_time = current_time;
        
        started = true;
    }
}

void i2c_master_init(i2c_master_bus_handle_t *bus_handle){
      i2c_master_bus_config_t bus_config = {
        .i2c_port = I2C_MASTER_NUM,
        .sda_io_num = I2C_SDA,
        .scl_io_num = I2C_SCL,
        .clk_source = I2C_CLK_SRC_DEFAULT,
        .glitch_ignore_cnt = 7,
        .flags.enable_internal_pullup = true,
    };
    ESP_ERROR_CHECK(i2c_new_master_bus(&bus_config, bus_handle));
}

#include <math.h>
#include <stdint.h>

#define WHEEL_DIAMETER_MM 70.0f
#define WHEEL_BASE_MM 200.0f
#define MICROSTEPPING 32.0f
#define STEPS_PER_REV (200.0f * MICROSTEPPING)
#define MOVE_COMPENSATION 1.0f
#define ROTATION_COMPENSATION 1.0f

#define DIR_FORWARD 1
#define DIR_LEFT 1
#define DIR_RIGHT 0

uint32_t calculateMoveSteps(float dx_mm, float dy_mm) {
    float distance_mm = sqrtf(dx_mm * dx_mm + dy_mm * dy_mm);
    float circumference_mm = (float)M_PI * WHEEL_DIAMETER_MM;
    float steps = (distance_mm / circumference_mm) * STEPS_PER_REV * MOVE_COMPENSATION;
    return (uint32_t)roundf(steps);
}

uint32_t calculateRotationSteps(float dx_mm, float dy_mm, float current_heading, float *new_heading_out, float *rot_angle_out) {
    float target_angle = atan2f(dy_mm, dx_mm);
    *new_heading_out = target_angle;
    
    float diff_angle = target_angle - current_heading;
    
    while (diff_angle > (float)M_PI) {
        diff_angle -= 2.0f * (float)M_PI;
    }
    while (diff_angle < -(float)M_PI) {
        diff_angle += 2.0f * (float)M_PI;
    }
    
    *rot_angle_out = diff_angle;
    
    float distance_wheel_mm = fabsf(diff_angle) * (WHEEL_BASE_MM / 2.0f);
    float circumference_mm = (float)M_PI * WHEEL_DIAMETER_MM;
    float steps = (distance_wheel_mm / circumference_mm) * STEPS_PER_REV * ROTATION_COMPENSATION;
    
    return (uint32_t)roundf(steps);
}

void navigateTo(float x, float y, uint32_t speedHz) {
    static uint8_t is_initialized = 0;
    static float current_x = 0.0f;
    static float current_y = 0.0f;
    static float current_heading = 0.0f;
    
    if (!is_initialized) {
        current_x = x;
        current_y = y;
        current_heading = 0.0f;
        is_initialized = 1;
        return;
    }
    
    float dx = x - current_x;
    float dy = y - current_y;
    
    if (dx == 0.0f && dy == 0.0f) {
        return;
    }
    
    float new_heading = 0.0f;
    float rot_angle = 0.0f;
    uint32_t rotSteps = calculateRotationSteps(dx, dy, current_heading, &new_heading, &rot_angle);
    
    if (rotSteps > 0) {
        uint8_t rotDir = (rot_angle >= 0.0f) ? DIR_LEFT : DIR_RIGHT;
        robotRotate(rotDir, speedHz, rotSteps);
    }
    
    uint32_t moveSteps = calculateMoveSteps(dx, dy);
    
    if (moveSteps > 0) {
        robotMove(DIR_FORWARD, speedHz, moveSteps);
    }
    
    current_x = x;
    current_y = y;
    current_heading = new_heading;
}

static void gcodeParser(FILE *program){
    char line[MAX_CHAR_SIZE];

    while(1){
        robot_state state = START;

        if(xSemaphoreTake(robot_state_mutex, pdMS_TO_TICKS(10))){
            state = current_state;
            xSemaphoreGive(robot_state_mutex);
        }

        if(state == STOP){
            return;
        }

        if(state == PAUSE){
            vTaskDelay(pdMS_TO_TICKS(200));
            continue;
        }

        if(!fgets(line, sizeof(line), program)){
            break;
        }

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

static void executeCmd(command *cmd){
    switch(cmd->num){
        case 0:
            navigateTo(cmd->a, cmd->b, 500);
            ESP_LOGI(TAG, "G00 a=%u b=%u", cmd->a, cmd->b);
            break;
        case 1:
            ESP_LOGI(TAG, "G01 tool=%u", cmd->tool);
            if (cmd->tool == 1){
                ESP_LOGI(TAG, "TOOL UP");
                servo_set_position(servo_tool, 130);
            }else{
                ESP_LOGI(TAG, "TOOL DOWN");
                servo_set_position(servo_tool, 0);
            }
            vTaskDelay(pdMS_TO_TICKS(200));
            break;
        case 3:
            ESP_LOGI(TAG, "G03 a=%u b=%u", cmd->a, cmd->b);
            servo_set_position(servo_base, cmd->a);
            servo_set_position(servo_arm, 180-cmd->b);
            vTaskDelay(pdMS_TO_TICKS(50));
            break;
        default:
            ESP_LOGE(TAG, "Command not found");
            break;
    }
} 

static void execute_gcode(void *pvParameters){
    const char* filename = (const char*)pvParameters;
    char program[256];

    while(1){
        if(started){
            if(xSemaphoreTake(sd_card_mutex, pdMS_TO_TICKS(1000))){
                snprintf(program, sizeof(program), "%s/%s", MOUNT_POINT, filename);
                
                ESP_LOGI(TAG, "%s", program);
                if (sd_read_file(program, &gcodeParser) != ESP_OK) {
                    
                }
                started = false;
                current_state = STOP;
                xSemaphoreGive(sd_card_mutex);
            }
        }else if(xSemaphoreTake(robot_state_mutex, pdMS_TO_TICKS(100))) {
            if(current_state == START){
                started = true;
                xSemaphoreGive(robot_state_mutex);
            } else {
                xSemaphoreGive(robot_state_mutex);
                vTaskDelay(pdMS_TO_TICKS(1000));
            }     
        }
    }
}

void manual_control_task(void *pvParameters) {
    mov_controls receivedCtrl;
    command cmd;

    while (1) {
        if (xQueueReceive(manual_mov_queue, &receivedCtrl, portMAX_DELAY) == pdPASS) {
            
            if (receivedCtrl.x_coord != 0 || receivedCtrl.y_coord != 0) {
                cmd.num = 0;
                cmd.a = (int16_t)receivedCtrl.x_coord;
                cmd.b = (int16_t)receivedCtrl.y_coord;
                cmd.tool = 0;
            } 
            else if (receivedCtrl.base_angle != 0 || receivedCtrl.arm_angle != 0) {
                cmd.num = 3;
                cmd.a = (uint16_t)receivedCtrl.base_angle;
                cmd.b = (uint16_t)receivedCtrl.arm_angle;
                cmd.tool = 0;
            } 
            else {
                cmd.num = 1;
                cmd.a = 0;
                cmd.b = 0;
                cmd.tool = (int16_t)receivedCtrl.tool_angle;
            }

            ESP_LOGI(TAG, "Parsed successfully: num=%d, a=%d, b=%d, tool=%d",
                cmd.num, 
                cmd.a, 
                cmd.b, 
                cmd.tool
            );
            executeCmd(&cmd);
            
        }
        vTaskDelay(500);
    }
}

static void read_accel_data(void *pvParameters){
    i2c_master_dev_handle_t dev_handle = (i2c_master_dev_handle_t)pvParameters;
    uint8_t data[2];
    while (1)
    {   
        if(xSemaphoreTake(imu_readings_mutex, pdMS_TO_TICKS(500)) == pdTRUE){
            esp_err_t readHErr = mpu6050_register_read(dev_handle, ACCEL_XOUT_H, data, 1);
            esp_err_t readLErr = mpu6050_register_read(dev_handle, ACCEL_XOUT_L, data+1, 1);
            if(readHErr == ESP_OK && readLErr == ESP_OK){
                imu_readings.x_accel = (data[0] << 8) | data[1];
                // ESP_LOGI(TAG, "X_accel: %f", imu_readings.x_accel*valTOms2);
            }
            readHErr = mpu6050_register_read(dev_handle, ACCEL_YOUT_H, data, 1);
            readLErr = mpu6050_register_read(dev_handle, ACCEL_YOUT_L, data+1, 1);
            if(readHErr == ESP_OK && readLErr == ESP_OK){
                imu_readings.y_accel = (data[0] << 8) | data[1];
                // ESP_LOGI(TAG, "Y_accel: %f", imu_readings.y_accel*valTOms2);
            }
            readHErr = mpu6050_register_read(dev_handle, ACCEL_ZOUT_H, data, 1);
            readLErr = mpu6050_register_read(dev_handle, ACCEL_ZOUT_L, data+1, 1);
            if(readHErr == ESP_OK && readLErr == ESP_OK){
                imu_readings.z_accel = (data[0] << 8) | data[1];
                // ESP_LOGI(TAG, "Z_accel: %f\n", imu_readings.z_accel*valTOms2);
            }
            xSemaphoreGive(imu_readings_mutex);
        }
        vTaskDelay(pdMS_TO_TICKS(2000));
    }
}

void create_server_task(void *pvParameters)
{
    const char* base_path = (const char*)pvParameters;

    while(1) {
        ESP_LOGI(TAG, "Attempting to connect to network...");
        /* This helper function configures Wi-Fi or Ethernet, as selected in menuconfig.
        * Read "Establishing Wi-Fi or Ethernet Connection" section in
        * examples/protocols/README.md for more information about this function.
        */ 
        if (example_connect() == ESP_OK) {
            ESP_LOGI(TAG, "Network connected! Starting file server.");
            ESP_ERROR_CHECK(start_file_server(MOUNT_POINT));
            ESP_LOGI(TAG, "File server started");
            break;
        } else {
            ESP_LOGW(TAG, "Connection failed. Retrying in 10 seconds...");
            vTaskDelay(pdMS_TO_TICKS(10000));
        }
    }
    // httpd_start creates its own task so this one is not needed
    vTaskDelete(NULL);
}

void gpio_monitor_task(void *pvParameters) {
    int last_state = -1;

    while (1) {
        int sensor_value = gpio_get_level(GPIO_NUM_9);
        
        // ESP_LOGI(TAG, "Wartosc na pinie 9: %d", sensor_value);

        if (sensor_value != last_state) {
            if (sensor_value == 1) {
                current_state = STOP;
                if (gcode_task_handle != NULL) vTaskSuspend(gcode_task_handle);
                if (manual_control_task_handle != NULL) vTaskSuspend(manual_control_task_handle);
                if (imu_task_handle != NULL) vTaskSuspend(imu_task_handle);
            } else if (sensor_value == 0 && last_state != -1) {
                if (gcode_task_handle != NULL) vTaskResume(gcode_task_handle);
                if (manual_control_task_handle != NULL) vTaskResume(manual_control_task_handle);
                if (imu_task_handle != NULL) vTaskResume(imu_task_handle);
            }
            last_state = sensor_value;
        }

        vTaskDelay(pdMS_TO_TICKS(200));
    }
}

// TODO: Ujednolicic error handling
void app_main(){
    esp_err_t ret;

    // Initialize sd card
    sdmmc_card_t *card = NULL;
    ret = card_init(&card);
    if (ret != ESP_OK) {
        return;
    }

    // Initialiaze servos
    servo_ledc_init(servo_base);
    servo_ledc_init(servo_arm);
    servo_ledc_init(servo_tool);

    // Initialize motors
    initSteppers();

    // GPIO Init
    gpio_config_t io_conf = {};
    io_conf.intr_type = GPIO_INTR_NEGEDGE;
    io_conf.mode = GPIO_MODE_INPUT;
    io_conf.pin_bit_mask = GPIO_INPUT_PIN_SEL;
    io_conf.pull_down_en = GPIO_PULLDOWN_DISABLE;
    io_conf.pull_up_en = GPIO_PULLUP_ENABLE;
    gpio_config(&io_conf);

    gpio_install_isr_service(ESP_INTR_FLAG_DEFAULT);
    gpio_isr_handler_add(BUTTON_GPIO, start_execution_isr, NULL);

    gpio_reset_pin(GPIO_NUM_9);
    gpio_set_direction(GPIO_NUM_9, GPIO_MODE_INPUT);
    gpio_set_pull_mode(GPIO_NUM_9, GPIO_PULLUP_ENABLE);

    // Initialize MPU6050 IMU
    i2c_master_bus_handle_t bus_handle;
    i2c_master_dev_handle_t dev_handle;
    i2c_master_init(&bus_handle);
    i2c_mpu6050_init(&bus_handle, &dev_handle);

    // Initialize file server components
    ESP_ERROR_CHECK(nvs_flash_init());
    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());

    // Create Mutexes
    robot_state_mutex = xSemaphoreCreateMutex();
    sd_card_mutex = xSemaphoreCreateMutex();
    imu_readings_mutex = xSemaphoreCreateMutex();
    manual_mov_queue = xQueueCreate(manual_mov_queue_len, sizeof(mov_controls));

    // Task Creation
    xTaskCreatePinnedToCore(create_server_task, "start_server", 4096, NULL, 10, NULL, CORE0);
    xTaskCreatePinnedToCore(execute_gcode, "execute_gcode", 4096, (void*)filename, 1, &gcode_task_handle, CORE1);
    xTaskCreatePinnedToCore(manual_control_task, "manual_robot_control", 4096, NULL, 2, &manual_control_task_handle, CORE1);
    xTaskCreatePinnedToCore(read_accel_data, "read_imu_data", 4096, (void*)dev_handle, 3, &imu_task_handle, CORE1);
    xTaskCreatePinnedToCore(gpio_monitor_task, "gpio_monitor", 2048, NULL, 10, NULL, CORE1);
}
