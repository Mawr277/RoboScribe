/**
 * @file MPU6050.h
 * @author Maciej Nowicki (maciejnowicki1234@gmail.com)
 * @brief Plik nagłówkowy do ułatwionej obsługi akcelerometru MPU6050
 * @version 1.00
 * @date 2026-11-01
 * @ingroup drivers
 * 
 * @copyright Copyright (c) 2026
 * 
 */
#include <stdio.h>
#include "driver/i2c_master.h"
#include "freertos/FreeRTOS.h"

#define I2C_MASTER_FREQ_HZ          400000
#define I2C_MASTER_TIMEOUT_MS       1000
#define MPU6050_RESET_BIT           7

/** 
 * @def MPU6050_ADDR_0 
 * @brief Adres I2C MPU6050 dla stanu 0 na pinie AIN0
 */
/** 
 * @def MPU6050_ADDR_1 
 * @brief Adres I2C MPU6050 dla stanu 1 na pinie AIN0
 */

#define MPU6050_ADDR_0 0x68 /// AIN0 - 0
#define MPU6050_ADDR_1 0x69 /// AIN0 - 1

/// @brief Wszystkie użyteczne adresy rejestrów MPU6050
typedef enum {
    GYRO_CONFIG  = 0x1B, /// Konfiguracja żyroskopu
    ACCEL_CONFIG = 0x1C, /// Konfiguracja akcelerometru
    FIFO_EN      = 0x23, /// Włączanie i konfiguracja rejestru FIFO

    INT_ENABLE   = 0x38, /// Włącz przerwania
    INT_STATUS   = 0x3A, /// Status przerwania

    ACCEL_XOUT_H = 0x3B, /// Przyśpieszenie osi X - bajt najbardziej znaczący
    ACCEL_XOUT_L = 0x3C, /// Przyśpieszenie osi X - bajt najmniej znaczący
    ACCEL_YOUT_H = 0x3D, /// Przyśpieszenie osi Y - bajt najbardziej znaczący
    ACCEL_YOUT_L = 0x3E, /// Przyśpieszenie osi Y - bajt najmniej znaczący
    ACCEL_ZOUT_H = 0x3F, /// Przyśpieszenie osi Z - bajt najbardziej znaczący
    ACCEL_ZOUT_L = 0x40, /// Przyśpieszenie osi Z - bajt najmniej znaczący
    TEMP_H       = 0x41, /// Pomiar temperatury - bajt najbardziej znaczący
    TEMP_L       = 0x42, /// Pomiar temperatury - bajt najmniej znaczący
    GYRO_XOUT_H  = 0x43, /// Prędkość kątowa X  - bajt najbardziej znaczący
    GYRO_XOUT_L  = 0x44, /// Prędkość kątowa X  - bajt najmniej znaczący
    GYRO_YOUT_H  = 0x45, /// Prędkość kątowa Y  - bajt najbardziej znaczący
    GYRO_YOUT_L  = 0x46, /// Prędkość kątowa Y  - bajt najmniej znaczący
    GYRO_ZOUT_H  = 0x47, /// Prędkość kątowa Z  - bajt najbardziej znaczący
    GYRO_ZOUT_L  = 0x48, /// Prędkość kątowa Z  - bajt najmniej znaczący

    USER_CTRL    = 0x6A, /// Sterownie
    PWR_MGMT_1   = 0x6B, /// Ustawienia zasilania 1
    PWR_MGMT_2   = 0x6C, /// Ustawienia zasilania 2
    FIFO_COUNTH  = 0x72, /// Licznik FIFO - bajt najbardziej znaczący
    FIFO_COUNTL  = 0x73, /// Licznik FIFO - bajt najmniej znaczący
    FIFO_R_W     = 0x74, /// Rejestr FIFO - odczyt/zapis
    WHO_AM_I     = 0x75, /// Identyfikacja sensoru
} mpu6050_reg_addr;

/**
 * @brief Inicjalizacja I2C (urządzeń master i slave) 
 * 
 * @param bus_handle wskaźnik uchwytu do obsługi szyny I2C
 * @param dev_handle wskaźnik uchwytu do obsługi urządzenia (slave)
 */
void i2c_mpu6050_init(i2c_master_bus_handle_t *bus_handle, i2c_master_dev_handle_t *dev_handle);

/**
 * @brief Odczytaj serie bajtów ze wskazanego rejestru MPU6050
 * 
 * @param dev_handle uchwyt do obsługi urządzenia (slave)
 * @param reg_addr adres rejestru do odczytu danych
 * @param data wskaźnik miejsca do zapisu danych
 * @param len długość danych w bajtach
 */
esp_err_t mpu6050_register_read(i2c_master_dev_handle_t dev_handle, mpu6050_reg_addr reg_addr, uint8_t *data, size_t len);

/**
 * @brief Zapisz bajt do wskazanego rejestru MPU6050
 * 
 * @param dev_handle uchwyt do obsługi urządzenia (slave)
 * @param reg_addr adres rejestru do zapisu danych
 * @param data wskaźnik miejsca do odczytu
 */
esp_err_t mpu6050_register_write(i2c_master_dev_handle_t dev_handle, mpu6050_reg_addr reg_addr, uint8_t data);