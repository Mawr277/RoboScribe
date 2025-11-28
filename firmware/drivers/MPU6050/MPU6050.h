#include <stdio.h>
#include "driver/i2c_master.h"
#include "freertos/FreeRTOS.h"

#define I2C_MASTER_FREQ_HZ          400000
#define I2C_MASTER_TIMEOUT_MS       1000
#define MPU6050_RESET_BIT           7

// Sensor I2C address depends on AIN0 pin state
#define MPU6050_ADDR_0 0x68 // AIN0 - 0
#define MPU6050_ADDR_1 0x69 // AIN0 - 1

typedef enum {
    GYRO_CONFIG  = 0x1B,
    ACCEL_CONFIG = 0x1C,
    FIFO_EN      = 0x23,
    I2C_MST_CTRL = 0x24,

    INT_ENABLE   = 0x38,
    INT_STATUS   = 0x3A,

    ACCEL_XOUT_H = 0x3B,
    ACCEL_XOUT_L = 0x3C,
    ACCEL_YOUT_H = 0x3D,
    ACCEL_YOUT_L = 0x3E,
    ACCEL_ZOUT_H = 0x3F,
    ACCEL_ZOUT_L = 0x40,
    TEMP_H       = 0x41,
    TEMP_L       = 0x42,
    GYRO_XOUT_H  = 0x43,
    GYRO_XOUT_L  = 0x44,
    GYRO_YOUT_H  = 0x45,
    GYRO_YOUT_L  = 0x46,
    GYRO_ZOUT_H  = 0x47,
    GYRO_ZOUT_L  = 0x48,

    USER_CTRL    = 0x6A,
    PWR_MGMT_1   = 0x6B,
    PWR_MGMT_2   = 0x6C,
    FIFO_COUNTH  = 0x72,
    FIFO_COUNTL  = 0x73,
    FIFO_R_W     = 0x74,
    WHO_AM_I     = 0x75,
} mpu6050_reg_addr;

void i2c_mpu6050_init(i2c_master_bus_handle_t *bus_handle, i2c_master_dev_handle_t *dev_handle);

esp_err_t mpu6050_register_read(i2c_master_dev_handle_t dev_handle, mpu6050_reg_addr reg_addr, uint8_t *data, size_t len);
esp_err_t mpu6050_register_write(i2c_master_dev_handle_t dev_handle, mpu6050_reg_addr reg_addr, uint8_t data);

esp_err_t read_acc_X();
