/**
 * @file mlx90640_api.c
 * @brief MLX90640 API mock implementation for unit testing
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
 * This is a mock implementation for unit testing that does not perform
 * actual sensor communication or calculations.
 * SPDX-FileCopyrightText: 2025 Heartland.Data Inc.
 * SPDX-License-Identifier: Apache-2.0
 */

#include "mlx90640_api.h"
#include <stdio.h>
#include <string.h>
#include <math.h>

/**
 * @brief Mock implementation of MLX90640_DumpEE
 */
int MLX90640_DumpEE(uint8_t slaveAddr, uint16_t *eeData)
{
    printf("[MOCK] MLX90640_DumpEE: addr=0x%02X\n", slaveAddr);

    if (eeData == NULL)
    {
        return -1;
    }

    // Fill with dummy EEPROM data
    for (int i = 0; i < 832; i++)
    {                           // MLX90640 EEPROM is 832 words
        eeData[i] = 0x5555 + i; // Dummy data
    }

    return 0; // Success
}

/**
 * @brief Mock implementation of MLX90640_GetFrameData
 */
int MLX90640_GetFrameData(uint8_t slaveAddr, uint16_t *frameData)
{
    printf("[MOCK] MLX90640_GetFrameData: addr=0x%02X\n", slaveAddr);

    if (frameData == NULL)
    {
        return -1;
    }

    // Fill with dummy frame data (768 pixels + additional data)
    for (int i = 0; i < 834; i++)
    {                                      // MLX90640 frame is 834 words
        frameData[i] = 0x3000 + (i % 100); // Dummy temperature-like data
    }

    return 0; // Success
}

/**
 * @brief Mock implementation of MLX90640_ExtractParameters
 */
int MLX90640_ExtractParameters(uint16_t *eeData, paramsMLX90640 *mlx90640)
{
    printf("[MOCK] MLX90640_ExtractParameters\n");

    if (eeData == NULL || mlx90640 == NULL)
    {
        return -1;
    }

    // Initialize with dummy calibration parameters
    memset(mlx90640, 0, sizeof(paramsMLX90640));

    // Set some realistic dummy values
    mlx90640->kVdd = 1000;
    mlx90640->vdd25 = 3300;
    mlx90640->KvPTAT = 0.002f;
    mlx90640->KtPTAT = 0.001f;
    mlx90640->vPTAT25 = 1800;
    mlx90640->alphaPTAT = 9.0f;
    mlx90640->gainEE = 100;
    mlx90640->tgc = 0.02f;
    mlx90640->resolutionEE = 18;
    mlx90640->KsTa = 0.001f;

    // Initialize arrays with dummy values
    for (int i = 0; i < 768; i++)
    {
        mlx90640->alpha[i] = 0.001f + (i * 0.000001f);
        mlx90640->offset[i] = 100 + i;
        mlx90640->kta[i] = 0.0001f;
        mlx90640->kv[i] = 0.1f;
    }

    return 0; // Success
}

/**
 * @brief Mock implementation of MLX90640_GetVdd
 */
float MLX90640_GetVdd(uint16_t *frameData, const paramsMLX90640 *params)
{
    printf("[MOCK] MLX90640_GetVdd\n");
    (void)frameData; // Suppress unused parameter warning
    (void)params;
    return 3.3f; // Mock VDD value
}

/**
 * @brief Mock implementation of MLX90640_GetTa
 */
float MLX90640_GetTa(uint16_t *frameData, const paramsMLX90640 *params)
{
    printf("[MOCK] MLX90640_GetTa\n");
    (void)frameData; // Suppress unused parameter warning
    (void)params;
    return 25.0f; // Mock ambient temperature in Celsius
}

/**
 * @brief Mock implementation of MLX90640_GetImage
 */
void MLX90640_GetImage(uint16_t *frameData, const paramsMLX90640 *params, float *result)
{
    printf("[MOCK] MLX90640_GetImage\n");
    (void)frameData; // Suppress unused parameter warning
    (void)params;

    if (result == NULL)
    {
        return;
    }

    // Fill with dummy temperature data (768 pixels)
    for (int i = 0; i < 768; i++)
    {
        result[i] = 25.0f + (float)(i % 50) * 0.1f; // Mock temperature range 25-30°C
    }
}

/**
 * @brief Mock implementation of MLX90640_CalculateTo
 */
void MLX90640_CalculateTo(uint16_t *frameData, const paramsMLX90640 *params, float emissivity, float tr, float *result)
{
    printf("[MOCK] MLX90640_CalculateTo: emissivity=%.2f, tr=%.2f\n", emissivity, tr);
    (void)frameData; // Suppress unused parameter warning
    (void)params;

    if (result == NULL)
    {
        return;
    }

    // Fill with dummy object temperature data (768 pixels)
    for (int i = 0; i < 768; i++)
    {
        result[i] = 30.0f + (float)(i % 100) * 0.05f; // Mock object temperature range 30-35°C
    }
}

/**
 * @brief Mock implementation of MLX90640_SetResolution
 */
int MLX90640_SetResolution(uint8_t slaveAddr, uint8_t resolution)
{
    printf("[MOCK] MLX90640_SetResolution: addr=0x%02X, resolution=%u\n", slaveAddr, resolution);
    return 0; // Success
}

/**
 * @brief Mock implementation of MLX90640_GetCurResolution
 */
int MLX90640_GetCurResolution(uint8_t slaveAddr)
{
    printf("[MOCK] MLX90640_GetCurResolution: addr=0x%02X\n", slaveAddr);
    return 18; // Mock 18-bit resolution
}

/**
 * @brief Mock implementation of MLX90640_SetRefreshRate
 */
int MLX90640_SetRefreshRate(uint8_t slaveAddr, uint8_t refreshRate)
{
    printf("[MOCK] MLX90640_SetRefreshRate: addr=0x%02X, rate=%u\n", slaveAddr, refreshRate);
    return 0; // Success
}

/**
 * @brief Mock implementation of MLX90640_GetRefreshRate
 */
int MLX90640_GetRefreshRate(uint8_t slaveAddr)
{
    printf("[MOCK] MLX90640_GetRefreshRate: addr=0x%02X\n", slaveAddr);
    return 8; // Mock 8Hz refresh rate
}

/**
 * @brief Mock implementation of MLX90640_GetSubPageNumber
 */
int MLX90640_GetSubPageNumber(uint16_t *frameData)
{
    printf("[MOCK] MLX90640_GetSubPageNumber\n");
    (void)frameData; // Suppress unused parameter warning
    return 0;        // Mock subpage 0
}

/**
 * @brief Mock implementation of MLX90640_GetCurMode
 */
int MLX90640_GetCurMode(uint8_t slaveAddr)
{
    printf("[MOCK] MLX90640_GetCurMode: addr=0x%02X\n", slaveAddr);
    return 1; // Mock chess mode
}

/**
 * @brief Mock implementation of MLX90640_SetInterleavedMode
 */
int MLX90640_SetInterleavedMode(uint8_t slaveAddr)
{
    printf("[MOCK] MLX90640_SetInterleavedMode: addr=0x%02X\n", slaveAddr);
    return 0; // Success
}

/**
 * @brief Mock implementation of MLX90640_SetChessMode
 */
int MLX90640_SetChessMode(uint8_t slaveAddr)
{
    printf("[MOCK] MLX90640_SetChessMode: addr=0x%02X\n", slaveAddr);
    return 0; // Success
}
