/**
 * @file mlx90640_api.h
 * @brief MLX90640 IR sensor API for ESP32-IDF
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

#ifndef _MLX90640_API_H_
#define _MLX90640_API_H_

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief MLX90640 parameters structure
 */
typedef struct {
    int16_t kVdd;
    int16_t vdd25;
    float KvPTAT;
    float KtPTAT;
    uint16_t vPTAT25;
    float alphaPTAT;
    int16_t gainEE;
    float tgc;
    float cpKv;
    float cpKta;
    uint8_t resolutionEE;
    uint8_t calibrationModeEE;
    float KsTa;
    float ksTo[4];
    int16_t ct[4];
    float alpha[768];
    int16_t offset[768];
    float kta[768];
    float kv[768];
    float cpAlpha[2];
    int16_t cpOffset[2];
    float ilChessC[3];
    uint16_t brokenPixels[5];
    uint16_t outlierPixels[5];
} paramsMLX90640;

/**
 * @brief MLX90640 API function declarations
 */
int MLX90640_DumpEE(uint8_t slaveAddr, uint16_t *eeData);
int MLX90640_GetFrameData(uint8_t slaveAddr, uint16_t *frameData);
int MLX90640_ExtractParameters(uint16_t *eeData, paramsMLX90640 *mlx90640);
float MLX90640_GetVdd(uint16_t *frameData, const paramsMLX90640 *params);
float MLX90640_GetTa(uint16_t *frameData, const paramsMLX90640 *params);
void MLX90640_GetImage(uint16_t *frameData, const paramsMLX90640 *params, float *result);
void MLX90640_CalculateTo(uint16_t *frameData, const paramsMLX90640 *params, float emissivity, float tr, float *result);
int MLX90640_SetResolution(uint8_t slaveAddr, uint8_t resolution);
int MLX90640_GetCurResolution(uint8_t slaveAddr);
int MLX90640_SetRefreshRate(uint8_t slaveAddr, uint8_t refreshRate);
int MLX90640_GetRefreshRate(uint8_t slaveAddr);
int MLX90640_GetSubPageNumber(uint16_t *frameData);
int MLX90640_GetCurMode(uint8_t slaveAddr);
int MLX90640_SetInterleavedMode(uint8_t slaveAddr);
int MLX90640_SetChessMode(uint8_t slaveAddr);

#ifdef __cplusplus
}
#endif

#endif /* _MLX90640_API_H_ */
