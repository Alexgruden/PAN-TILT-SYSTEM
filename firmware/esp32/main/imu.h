#ifndef IMU_H
#define IMU_H

#include <stdio.h>
#include <stdint.h>
#include "esp_err.h"
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
 * @brief Read data from the IMU sensor.
 * 
 * This function reads the accelerometer and gyroscope data from the IMU sensor and fills the provided IMUData structure with the readings.
 * 
 * @param[out] readings Pointer to an IMUData structure where the sensor readings will be stored.
 */
esp_err_t imu_read(IMUData *readings);

/**
 * @brief Initialize the IMU sensor.
 * 
 * This function initializes the IMU sensor by setting up the I2C bus and device handle, and configuring the sensor's registers as needed.
 */
void imu_init(void);

#endif // IMU_H