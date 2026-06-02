/**
 * @file mlx90640_i2c_driver.c
 * @brief MLX90640 I2C driver implementation for ESP32-IDF
 * @copyright (C) 2017 Melexis N.V.
 * @copyright (C) 2025 Heartland.Data Inc.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 * Based on the Melexis MLX90640 library:
 *   https://github.com/melexis/mlx90640-library
 * Adapted for Arduino by SparkFun Electronics:
 *   https://github.com/sparkfun/Qwiic_IR_Array_MLX90640
 * Ported to ESP32-IDF and modified by Heartland.Data Inc.
 * SPDX-FileCopyrightText: 2017 Melexis N.V.
 * SPDX-FileCopyrightText: 2025 Heartland.Data Inc.
 * SPDX-License-Identifier: Apache-2.0
 */

#include "mlx90640_i2c_driver.h"
#include "esp_log.h"
#include "esp_err.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/i2c_master.h"

static const char *TAG = "MLX90640_I2C";

static i2c_master_bus_handle_t s_bus_handle = NULL;
static i2c_master_dev_handle_t s_dev_handle = NULL;
static bool is_initialized = false;

/**
 * @brief Initialize I2C driver for MLX90640
 */
int MLX90640_I2CInit(i2c_port_t port, int sda_pin, int scl_pin, uint32_t freq_hz, uint8_t device_address)
{
    if (is_initialized) {
        ESP_LOGW(TAG, "I2C already initialized");
        return ESP_OK;
    }

    // Initialize I2C master bus
    i2c_master_bus_config_t bus_config = {
        .i2c_port = port,
        .sda_io_num = sda_pin,
        .scl_io_num = scl_pin,
        .clk_source = I2C_CLK_SRC_DEFAULT,
        .glitch_ignore_cnt = 7,
        .flags.enable_internal_pullup = true,
    };

    esp_err_t err = i2c_new_master_bus(&bus_config, &s_bus_handle);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Failed to create I2C master bus: %s", esp_err_to_name(err));
        return -1;
    }

    // Add MLX90640 device to the bus
    i2c_device_config_t dev_config = {
        .dev_addr_length = I2C_ADDR_BIT_LEN_7,
        .device_address = device_address,
        .scl_speed_hz = freq_hz,
    };

    err = i2c_master_bus_add_device(s_bus_handle, &dev_config, &s_dev_handle);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Failed to add MLX90640 device (0x%02X): %s", device_address, esp_err_to_name(err));
        i2c_del_master_bus(s_bus_handle);
        s_bus_handle = NULL;
        return -1;
    }

    is_initialized = true;
    ESP_LOGI(TAG, "I2C initialized successfully on port %d with device 0x%02X", port, device_address);
    return ESP_OK;
}

/**
 * @brief Deinitialize I2C driver
 */
int MLX90640_I2CDeinit(i2c_port_t port)
{
    if (!is_initialized) {
        ESP_LOGW(TAG, "I2C not initialized");
        return ESP_OK;
    }

    esp_err_t err = ESP_OK;

    // Remove device from bus
    if (s_dev_handle != NULL) {
        err = i2c_master_bus_rm_device(s_dev_handle);
        if (err != ESP_OK) {
            ESP_LOGE(TAG, "Failed to remove device from I2C bus: %s", esp_err_to_name(err));
        }
        s_dev_handle = NULL;
    }

    // Delete master bus
    if (s_bus_handle != NULL) {
        err = i2c_del_master_bus(s_bus_handle);
        if (err != ESP_OK) {
            ESP_LOGE(TAG, "Failed to delete I2C master bus: %s", esp_err_to_name(err));
            return -1;
        }
        s_bus_handle = NULL;
    }

    is_initialized = false;
    ESP_LOGI(TAG, "I2C deinitialized successfully");
    return ESP_OK;
}

/**
 * @brief Read data from MLX90640 via I2C
 */
int MLX90640_I2CRead(uint8_t deviceAddress, unsigned int startAddress, unsigned int nWordsRead, uint16_t *data)
{
    if (!is_initialized || s_dev_handle == NULL) {
        ESP_LOGE(TAG, "I2C not initialized");
        return -1;
    }

    if (data == NULL || nWordsRead == 0) {
        ESP_LOGE(TAG, "Invalid parameters");
        return -1;
    }

    uint8_t write_buf[2];
    write_buf[0] = (startAddress >> 8) & 0xFF;  // High byte
    write_buf[1] = startAddress & 0xFF;         // Low byte

    // Read data (each word is 2 bytes)
    uint8_t *read_buf = (uint8_t *)data;
    size_t read_size = nWordsRead * 2;

    // Use write-read transaction for register access
    esp_err_t err = i2c_master_transmit_receive(s_dev_handle, write_buf, 2, read_buf, read_size, 1000);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Failed to read data: %s", esp_err_to_name(err));
        return -1;
    }

    // Convert from big-endian to little-endian
    for (unsigned int i = 0; i < nWordsRead; i++) {
        uint8_t temp = read_buf[i * 2];
        read_buf[i * 2] = read_buf[i * 2 + 1];
        read_buf[i * 2 + 1] = temp;
    }

    return 0;
}

/**
 * @brief Write data to MLX90640 via I2C
 */
int MLX90640_I2CWrite(uint8_t deviceAddress, unsigned int writeAddress, uint16_t data)
{
    if (!is_initialized || s_dev_handle == NULL) {
        ESP_LOGE(TAG, "I2C not initialized");
        return -1;
    }

    uint8_t write_buf[4];
    write_buf[0] = (writeAddress >> 8) & 0xFF;  // Address high byte
    write_buf[1] = writeAddress & 0xFF;         // Address low byte
    write_buf[2] = (data >> 8) & 0xFF;          // Data high byte
    write_buf[3] = data & 0xFF;                 // Data low byte

    esp_err_t err = i2c_master_transmit(s_dev_handle, write_buf, 4, 1000);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Failed to write data: %s", esp_err_to_name(err));
        return -1;
    }

    return 0;
}

/**
 * @brief Set I2C frequency
 */
void MLX90640_I2CFreqSet(uint32_t freq_hz)
{
    if (!is_initialized) {
        ESP_LOGW(TAG, "I2C not initialized, cannot set frequency");
        return;
    }

    // Note: In ESP32-IDF, changing frequency at runtime requires reconfiguration
    ESP_LOGI(TAG, "Frequency set to %ld Hz (requires reinitialization)", freq_hz);
}
