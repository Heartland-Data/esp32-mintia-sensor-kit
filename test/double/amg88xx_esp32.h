/**
 * @file amg88xx_esp32.h
 * @brief AMG88xx テストダブル 実装
 *
 * @copyright Copyright (C) 2026, Heartland Data inc. All Rights Reserved.
 * SPDX-FileCopyrightText: 2026 Heartland Data inc.
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef AMG88XX_ESP32_H
#define AMG88XX_ESP32_H

#include <stdint.h>
#include <stdbool.h>
#include "esp_err.h"
#include "driver/i2c_master.h"

#ifdef __cplusplus
extern "C"
{
#endif

/*=========================================================================
    I2C ADDRESS/BITS
    -----------------------------------------------------------------------*/
#define AMG88XX_ADDRESS (0x68)
/*=========================================================================*/

/*=========================================================================
    REGISTERS
    -----------------------------------------------------------------------*/
#define AMG88XX_PCTL 0x00
#define AMG88XX_RST 0x01
#define AMG88XX_FPSC 0x02
#define AMG88XX_INTC 0x03
#define AMG88XX_STAT 0x04
#define AMG88XX_SCLR 0x05
// 0x06 reserved
#define AMG88XX_AVE 0x07
#define AMG88XX_INTHL 0x08
#define AMG88XX_INTHH 0x09
#define AMG88XX_INTLL 0x0A
#define AMG88XX_INTLH 0x0B
#define AMG88XX_IHYSL 0x0C
#define AMG88XX_IHYSH 0x0D
#define AMG88XX_TTHL 0x0E
#define AMG88XX_TTHH 0x0F
#define AMG88XX_INT_OFFSET 0x10
#define AMG88XX_PIXEL_OFFSET 0x80

    typedef enum
    {
        AMG88XX_NORMAL_MODE = 0x00,
        AMG88XX_SLEEP_MODE = 0x01,
        AMG88XX_STAND_BY_60 = 0x20,
        AMG88XX_STAND_BY_10 = 0x21
    } amg88xx_power_mode_t;

    typedef enum
    {
        AMG88XX_FLAG_RESET = 0x30,
        AMG88XX_INITIAL_RESET = 0x3F
    } amg88xx_sw_reset_t;

    typedef enum
    {
        AMG88XX_FPS_10 = 0x00,
        AMG88XX_FPS_1 = 0x01
    } amg88xx_frame_rate_t;

    typedef enum
    {
        AMG88XX_INT_DISABLED = 0x00,
        AMG88XX_INT_ENABLED = 0x01
    } amg88xx_int_enable_t;

    typedef enum
    {
        AMG88XX_DIFFERENCE = 0x00,
        AMG88XX_ABSOLUTE_VALUE = 0x01
    } amg88xx_int_mode_t;

    /*=========================================================================*/

#define AMG88XX_PIXEL_ARRAY_SIZE 64
#define AMG88XX_PIXEL_TEMP_CONVERSION 0.25f
#define AMG88XX_THERMISTOR_CONVERSION 0.0625f

    /**
     * @brief AMG88xx configuration structure
     */
    typedef struct
    {
        i2c_master_dev_handle_t dev_handle; ///< I2C device handle
        int timeout_ms;                     ///< I2C timeout in milliseconds
    } amg88xx_config_t;

    /**
     * @brief AMG88xx handle structure
     */
    typedef struct
    {
        amg88xx_config_t config; ///< Device configuration
        bool initialized;        ///< Initialization status
    } amg88xx_handle_t;

    esp_err_t amg88xx_init(amg88xx_handle_t *handle, const amg88xx_config_t *config);
    esp_err_t amg88xx_deinit(amg88xx_handle_t *handle);
    esp_err_t amg88xx_read_pixels_raw(amg88xx_handle_t *handle, uint8_t *buf, uint8_t pixels);
    esp_err_t amg88xx_read_pixels(amg88xx_handle_t *handle, float *buf, uint8_t pixels);
    esp_err_t amg88xx_read_thermistor(amg88xx_handle_t *handle, float *temperature);
    esp_err_t amg88xx_set_moving_average_mode(amg88xx_handle_t *handle, bool enable);
    esp_err_t amg88xx_enable_interrupt(amg88xx_handle_t *handle);
    esp_err_t amg88xx_disable_interrupt(amg88xx_handle_t *handle);
    esp_err_t amg88xx_set_interrupt_mode(amg88xx_handle_t *handle, amg88xx_int_mode_t mode);
    esp_err_t amg88xx_get_interrupt(amg88xx_handle_t *handle, uint8_t *buf, uint8_t size);
    esp_err_t amg88xx_clear_interrupt(amg88xx_handle_t *handle);
    esp_err_t amg88xx_set_interrupt_levels(amg88xx_handle_t *handle, float high, float low);
    esp_err_t amg88xx_set_interrupt_levels_manual(amg88xx_handle_t *handle, float high, float low, float hysteresis);
    esp_err_t amg88xx_set_power_mode(amg88xx_handle_t *handle, amg88xx_power_mode_t mode);
    esp_err_t amg88xx_set_frame_rate(amg88xx_handle_t *handle, amg88xx_frame_rate_t rate);
    esp_err_t amg88xx_software_reset(amg88xx_handle_t *handle, amg88xx_sw_reset_t reset_type);

#ifdef __cplusplus
}
#endif

#endif // AMG88XX_ESP32_H
