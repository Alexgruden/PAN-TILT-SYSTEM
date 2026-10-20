#include "imu.h"
#include "esp_err.h"
#include "sdkconfig.h"
#include "esp_log.h"
#include "driver/i2c_master.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"


static const char *TAG = "IMU";

#define I2C_MASTER_SCL_IO           22       /*!< GPIO number used for I2C master clock */
#define I2C_MASTER_SDA_IO           21       /*!< GPIO number used for I2C master data  */
#define I2C_MASTER_NUM              I2C_NUM_0                   /*!< I2C port number for master dev */
#define I2C_MASTER_FREQ_HZ          400000 /*!< I2C master clock frequency */
#define I2C_MASTER_TX_BUF_DISABLE   0                           /*!< I2C master doesn't need buffer */
#define I2C_MASTER_RX_BUF_DISABLE   0                           /*!< I2C master doesn't need buffer */
#define I2C_MASTER_TIMEOUT_MS       1000

#define MPU9250_SENSOR_ADDR         0x68        /*!< Address of the MPU9250 sensor */
#define MPU9250_WHO_AM_I_REG_ADDR   0x75        /*!< Register addresses of the "who am I" register */
#define MPU9250_PWR_MGMT_1_REG_ADDR 0x6B        /*!< Register addresses of the power management register */
#define MPU9250_ACCEL_XOUT_H_REG_ADDR 0x3B    /*!< Register addresses of the accelerometer data registers */
#define MPU9250_RESET_BIT           7
#define IMU_READ_LEN                 14          /*!< Number of bytes to read from the IMU sensor */ 

#define ACCEL_Z_BIAS 1460

static i2c_master_bus_handle_t s_bus_handle;
static i2c_master_dev_handle_t s_dev_handle;

static int32_t gyro_x_avg = 0;
static int32_t gyro_z_avg = 0;


/**
 * @brief Read a sequence of bytes from a MPU9250 sensor registers
 */
static esp_err_t mpu9250_register_read(i2c_master_dev_handle_t dev_handle, uint8_t reg_addr, uint8_t *data, size_t len)
{
    return i2c_master_transmit_receive(dev_handle, &reg_addr, 1, data, len, I2C_MASTER_TIMEOUT_MS);
}

/**
 * @brief Write a byte to a MPU9250 sensor register
 */
static esp_err_t mpu9250_register_write_byte(i2c_master_dev_handle_t dev_handle, uint8_t reg_addr, uint8_t data)
{
    uint8_t write_buf[2] = {reg_addr, data};
    return i2c_master_transmit(dev_handle, write_buf, sizeof(write_buf), I2C_MASTER_TIMEOUT_MS);
}

/**
 * @brief i2c master initialization
 */
static void i2c_master_init(i2c_master_bus_handle_t *bus_handle, i2c_master_dev_handle_t *dev_handle)
{
    i2c_master_bus_config_t bus_config = {
        .i2c_port = I2C_MASTER_NUM,
        .sda_io_num = I2C_MASTER_SDA_IO,
        .scl_io_num = I2C_MASTER_SCL_IO,
        .clk_source = I2C_CLK_SRC_DEFAULT,
        .glitch_ignore_cnt = 7,
        .flags.enable_internal_pullup = true,
    };
    ESP_ERROR_CHECK(i2c_new_master_bus(&bus_config, bus_handle));

    i2c_device_config_t dev_config = {
        .dev_addr_length = I2C_ADDR_BIT_LEN_7,
        .device_address = MPU9250_SENSOR_ADDR,
        .scl_speed_hz = I2C_MASTER_FREQ_HZ,
    };
    ESP_ERROR_CHECK(i2c_master_bus_add_device(*bus_handle, &dev_config, dev_handle));
}

void imu_init(void) {
    i2c_master_init(&s_bus_handle, &s_dev_handle);
    ESP_LOGI(TAG, "I2C initialized successfully");

    // Reset to known start state.
    ESP_ERROR_CHECK(mpu9250_register_write_byte(s_dev_handle, MPU9250_PWR_MGMT_1_REG_ADDR, 1 << MPU9250_RESET_BIT));

    vTaskDelay(pdMS_TO_TICKS(100)); // Delay for 100 ms to allow the reset to complete

    //Wake up the MPU9250 by clearing the sleep bit in the power management register
    ESP_ERROR_CHECK(mpu9250_register_write_byte(s_dev_handle, MPU9250_PWR_MGMT_1_REG_ADDR, 0x01));
}

esp_err_t imu_read(IMUData *readings) {
    uint8_t data[IMU_READ_LEN] = {0};
    esp_err_t ret = mpu9250_register_read(s_dev_handle, MPU9250_ACCEL_XOUT_H_REG_ADDR, data, IMU_READ_LEN);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to read IMU data: %s", esp_err_to_name(ret));
        return ret;
    }

    readings->ax = (int16_t)((data[0] << 8) | data[1]);
    readings->ay = (int16_t)((data[2] << 8) | data[3]);
    readings->az = ((int16_t)((data[4] << 8) | data[5]) - ACCEL_Z_BIAS);
    readings->gx = ((int16_t)((data[8] << 8) | data[9]) - gyro_x_avg);
    readings->gy = (int16_t)((data[10] << 8) | data[11]);
    readings->gz = ((int16_t)((data[12] << 8) | data[13]) - gyro_z_avg);

    return ESP_OK;
}
/** 
 * @brief Calibrate the IMU sensor.
 * 
 * Log “hold still,” take 100 readings 20 ms apart (2 seconds), and average gyro X and gyro Z.
*/
void imu_calibrate(void) {
    ESP_LOGI(TAG, "Hold still!!!!!!");

    gyro_x_avg = 0;
    gyro_z_avg = 0;
    int32_t gx_sum = 0;
    int32_t gz_sum = 0;
    uint8_t count = 0;

    for (int i = 0; i < 100; i++) {
        IMUData readings; 
        if (imu_read(&readings) == ESP_OK) {
            gx_sum += readings.gx;
            gz_sum += readings.gz;
            count++;
        }
        vTaskDelay(pdMS_TO_TICKS(20)); // Delay for 20 ms between readings
    }

    if (count == 0) {
        ESP_LOGE(TAG, "No valid IMU readings for calibration");
        return;
    }
    gyro_x_avg = gx_sum / count;
    gyro_z_avg = gz_sum / count;
}