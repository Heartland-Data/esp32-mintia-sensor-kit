/**
 * @file mlx90640_i2c_driver.h
 * @brief MLX90640 I2C driver for ESP32-IDF
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
 * Original code based on Melexis MLX90640 library.
 * Extensively modified and ported to ESP32-IDF by Heartland.Data Inc.
 * SPDX-FileCopyrightText: 2017 Melexis N.V.
 * SPDX-FileCopyrightText: 2025 Heartland.Data Inc.
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef _MLX90640_I2C_DRIVER_H_
#define _MLX90640_I2C_DRIVER_H_

#include <stdint.h>
#include "driver/i2c_master.h"

#ifdef __cplusplus
extern "C"
{
#endif

/**
 * @brief Default I2C configuration for MLX90640
 */
#define MLX90640_I2C_PORT I2C_NUM_0
#define MLX90640_I2C_SDA_PIN 21
#define MLX90640_I2C_SCL_PIN 22
#define MLX90640_I2C_FREQ_HZ 400000 // 400kHz
#define MLX90640_DEFAULT_ADDR 0x33  // Default 7-bit address

    /**
     * @brief Initialize I2C driver for MLX90640
     * @param port I2C port number
     * @param sda_pin SDA pin number
     * @param scl_pin SCL pin number
     * @param freq_hz I2C frequency in Hz
     * @param device_address Device I2C address (7-bit)
     * @return ESP_OK on success, error code otherwise
     */
    int MLX90640_I2CInit(i2c_port_t port, int sda_pin, int scl_pin, uint32_t freq_hz, uint8_t device_address);

    /**
     * @brief Deinitialize I2C driver
     * @param port I2C port number
     * @return ESP_OK on success, error code otherwise
     */
    int MLX90640_I2CDeinit(i2c_port_t port);

    /**
     * @brief Read data from MLX90640 via I2C
     * @param deviceAddress Device I2C address
     * @param startAddress Register start address
     * @param nWordsRead Number of 16-bit words to read
     * @param data Buffer to store read data
     * @return 0 on success, negative error code otherwise
     */
    int MLX90640_I2CRead(uint8_t deviceAddress, unsigned int startAddress, unsigned int nWordsRead, uint16_t *data);

    /**
     * @brief Write data to MLX90640 via I2C
     * @param deviceAddress Device I2C address
     * @param writeAddress Register address to write
     * @param data Data to write (16-bit)
     * @return 0 on success, negative error code otherwise
     */
    int MLX90640_I2CWrite(uint8_t deviceAddress, unsigned int writeAddress, uint16_t data);

    /**
     * @brief Set I2C frequency
     * @param freq_hz Frequency in Hz
     */
    void MLX90640_I2CFreqSet(uint32_t freq_hz);

#ifdef __cplusplus
}
#endif

#endif /* _MLX90640_I2C_DRIVER_H_ */
