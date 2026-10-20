#ifndef IMU_H
#define IMU_H

#include <stdio.h>
#include "sdkconfig.h"
#include "esp_log.h"
#include "driver/i2c_master.h"

/**
 * @brief IMU data structure to hold accelerometer and gyroscope readings.
 * 
 * This structure contains six 16-bit signed integers representing the accelerometer and gyroscope readings along the x, y, and z axes.
 */
typedef struct {
    int16_t ax;
    int16_t ay;
    int16_t az;
    int16_t gx;
    int16_t gy;
    int16_t gz;
} IMUData;

/**
 * @brief Initialize the I2C bus and device handle for the IMU.
 * 
 * @param[in] bus_handle Pointer to the I2C master bus handle to initialize.
 * @param[in] dev_handle Pointer to the I2C master device handle to initialize.
 */
void i2c_initalize(i2c_master_bus_handle_t *bus_handle, i2c_master_dev_handle_t *dev_handle);

/**
 * @brief Read data from the IMU over I2C.
 * 
 * This function reads a specified number of bytes from a given register address of the IMU device using the provided I2C device handle.
 * 
 * @param[in] dev_handle The I2C master device handle for the IMU.
 * @param[in] reg_addr The register address to read from.
 * @param[out] data Pointer to the buffer where the read data will be stored.
 * @param[in] len The number of bytes to read.
 * @return esp_err_t Returns ESP_OK on success, or an error code on failure.
 */
esp_err_t imu_read(i2c_master_dev_handle_t dev_handle, uint8_t reg_addr, uint8_t *data, size_t len);

/**
 * @brief Write a byte to a MPU9250 sensor register
 */
static esp_err_t mpu9250_register_write_byte(i2c_master_dev_handle_t dev_handle, uint8_t reg_addr, uint8_t data);

/**
 * @brief Initialize the IMU sensor.
 * 
 * This function initializes the IMU sensor by setting up the I2C bus and device handle, and configuring the sensor's registers as needed.
 */
imu_init(void);

#endif // IMU_H