#include "MPU6050.h"

/**
 * @brief Initialize I2C master and device
 */
void i2c_mpu6050_init(i2c_master_bus_handle_t *bus_handle, i2c_master_dev_handle_t *dev_handle)
{
    i2c_device_config_t dev_config = {
        .dev_addr_length = I2C_ADDR_BIT_LEN_7,
        .device_address = MPU6050_ADDR_0,
        .scl_speed_hz = I2C_MASTER_FREQ_HZ,
    };
    ESP_ERROR_CHECK(i2c_master_bus_add_device(*bus_handle, &dev_config, dev_handle));
    // Turns off sleep mode just in case
    ESP_ERROR_CHECK(mpu6050_register_write(*dev_handle, PWR_MGMT_1, 0 << MPU6050_RESET_BIT)); 
}


/**
 * @brief Read a sequence of bytes from a MPU9250 sensor registers
 */
esp_err_t mpu6050_register_read(i2c_master_dev_handle_t dev_handle, mpu6050_reg_addr reg_addr, uint8_t *data, size_t len)
{
    uint8_t write_buf = reg_addr;
    return i2c_master_transmit_receive(dev_handle, &write_buf, 1, data, len, I2C_MASTER_TIMEOUT_MS / portTICK_PERIOD_MS);
}

/**
 * @brief Write a byte to a MPU9250 sensor register
 */
esp_err_t mpu6050_register_write(i2c_master_dev_handle_t dev_handle, mpu6050_reg_addr reg_addr, uint8_t data)
{
    uint8_t write_buf[2] = {reg_addr, data};
    return i2c_master_transmit(dev_handle, write_buf, sizeof(write_buf), I2C_MASTER_TIMEOUT_MS / portTICK_PERIOD_MS);
}