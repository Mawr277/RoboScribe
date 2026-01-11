#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/i2c_master.h"

#include "MPU6050.h"


#define I2C_SDA                     5
#define I2C_SCL                     6
#define I2C_MASTER_NUM              I2C_NUM_0

void i2c_master_init(i2c_master_bus_handle_t *bus_handle){
      i2c_master_bus_config_t bus_config = {
        .i2c_port = I2C_MASTER_NUM,
        .sda_io_num = I2C_SDA,
        .scl_io_num = I2C_SCL,
        .clk_source = I2C_CLK_SRC_DEFAULT,
        .glitch_ignore_cnt = 7,
        .flags.enable_internal_pullup = false,
    };
    ESP_ERROR_CHECK(i2c_new_master_bus(&bus_config, bus_handle));
}

void app_main(void)
{
    uint8_t data[2];
    i2c_master_bus_handle_t bus_handle;
    i2c_master_dev_handle_t dev_handle;
    i2c_master_init(&bus_handle);
    i2c_mpu6050_init(&bus_handle, &dev_handle);

    while (1) {
        esp_err_t readHErr = mpu6050_register_read(dev_handle, ACCEL_XOUT_H, data, 1);
        esp_err_t readLErr = mpu6050_register_read(dev_handle, ACCEL_XOUT_L, data+1, 1);
        if(readHErr == ESP_OK && readLErr == ESP_OK){
            uint16_t value = (data[0] << 8) | data[1];
            printf("X Acceleration: %d \n", value);
        }
        readHErr = mpu6050_register_read(dev_handle, ACCEL_YOUT_H, data, 1);
        readLErr = mpu6050_register_read(dev_handle, ACCEL_YOUT_L, data+1, 1);
        if(readHErr == ESP_OK && readLErr == ESP_OK){
            uint16_t value = (data[0] << 8) | data[1];
            printf("Y Acceleration: %d \n\n", value);
        }
        vTaskDelay(pdMS_TO_TICKS(1000));
    }

    ESP_ERROR_CHECK(i2c_master_bus_rm_device(dev_handle));
    ESP_ERROR_CHECK(i2c_del_master_bus(bus_handle));
}