/**
 * @file mlx90640_i2c_driver.c
 * @brief MLX90640 I2C driver mock implementation for unit testing
 * @copyright (C) 2025 Heartland.Data Inc.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *fng, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 * This is a mock implementation for unit testing that does not perform
 * actual I2C communication.
 * SPDX-FileCopyrightText: 2025 Heartland.Data Inc.
 * SPDX-License-Identifier: Apache-2.0
 */

#include "mlx90640_i2c_driver.h"
#include <stdio.h>
#include <string.h>

/**
 * @brief Mock implementation of MLX90640_I2CInit
 */
int MLX90640_I2CInit(i2c_port_t port, int sda_pin, int scl_pin, uint32_t freq_hz, uint8_t device_address)
{
    printf("[MOCK] MLX90640_I2CInit: port=%d, sda=%d, scl=%d, freq=%u, addr=0x%02X\n",
           (int)port, sda_pin, scl_pin, freq_hz, device_address);
    return 0; // Success
}

/**
 * @brief Mock implementation of MLX90640_I2CDeinit
 */
int MLX90640_I2CDeinit(i2c_port_t port)
{
    printf("[MOCK] MLX90640_I2CDeinit: port=%d\n", (int)port);
    return 0; // Success
}

/**
 * @brief Mock implementation of MLX90640_I2CRead
 */
int MLX90640_I2CRead(uint8_t deviceAddress, unsigned int startAddress, unsigned int nWordsRead, uint16_t *data)
{
    printf("[MOCK] MLX90640_I2CRead: addr=0x%02X, start=0x%04X, words=%u\n",
           deviceAddress, startAddress, nWordsRead);

    if (data == NULL)
    {
        return -1;
    }

    // Fill with dummy data
    for (unsigned int i = 0; i < nWordsRead; i++)
    {
        data[i] = 0x1234 + i; // Dummy data
    }

    return 0; // Success
}

/**
 * @brief Mock implementation of MLX90640_I2CWrite
 */
int MLX90640_I2CWrite(uint8_t deviceAddress, unsigned int writeAddress, uint16_t data)
{
    printf("[MOCK] MLX90640_I2CWrite: addr=0x%02X, reg=0x%04X, data=0x%04X\n",
           deviceAddress, writeAddress, data);
    return 0; // Success
}

/**
 * @brief Mock implementation of MLX90640_I2CFreqSet
 */
void MLX90640_I2CFreqSet(uint32_t freq_hz)
{
    printf("[MOCK] MLX90640_I2CFreqSet: freq=%u Hz\n", freq_hz);
}
