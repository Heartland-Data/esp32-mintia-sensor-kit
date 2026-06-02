/**
 * @file amg88xx_esp32.c
 * @brief AMG88xx テストダブル 実装
 * @copyright Copyright (C) 2026, Heartland Data inc. All Rights Reserved.
 * SPDX-FileCopyrightText: 2026 Heartland Data inc.
 * SPDX-License-Identifier: Apache-2.0
 */

#include "string.h"
#include "amg88xx_esp32.h"

esp_err_t amg88xx_init(amg88xx_handle_t *handle, const amg88xx_config_t *config) { return ESP_OK; }
esp_err_t amg88xx_deinit(amg88xx_handle_t *handle) { return ESP_OK; }
esp_err_t amg88xx_read_pixels_raw(amg88xx_handle_t *handle, uint8_t *buf, uint8_t pixels) { return ESP_OK; }

esp_err_t amg88xx_read_pixels(amg88xx_handle_t *handle, float *buf, uint8_t pixels)
{
    float temperature[64] = {
        25.5, 26.0, 26.5, 27.0, 27.5, 28.0, 28.5, 29.0,
        25.0, 25.5, 26.0, 26.5, 27.0, 27.5, 28.0, 28.5,
        24.5, 25.0, 25.5, 26.0, 26.5, 27.0, 27.5, 28.0,
        24.0, 24.5, 25.0, 25.5, 26.0, 26.5, 27.0, 27.5,
        23.5, 24.0, 24.5, 25.0, 25.5, 26.0, 26.5, 27.0,
        23.0, 23.5, 24.0, 24.5, 25.0, 25.5, 26.0, 26.5,
        22.5, 23.0, 23.5, 24.0, 24.5, 25.0, 25.5, 26.0,
        22.0, 22.5, 23.0, 23.5, 24.0, 24.5, 25.0, 25.5};
    memcpy(buf, temperature, sizeof(temperature));
    return ESP_OK;
}

esp_err_t amg88xx_read_thermistor(amg88xx_handle_t *handle, float *temperature)
{
    *temperature = 25.5;
    return ESP_OK;
}

esp_err_t amg88xx_set_moving_average_mode(amg88xx_handle_t *handle, bool enable) { return ESP_OK; }
esp_err_t amg88xx_enable_interrupt(amg88xx_handle_t *handle) { return ESP_OK; }
esp_err_t amg88xx_disable_interrupt(amg88xx_handle_t *handle) { return ESP_OK; }
esp_err_t amg88xx_set_interrupt_mode(amg88xx_handle_t *handle, amg88xx_int_mode_t mode) { return ESP_OK; }
esp_err_t amg88xx_get_interrupt(amg88xx_handle_t *handle, uint8_t *buf, uint8_t size) { return ESP_OK; }
esp_err_t amg88xx_clear_interrupt(amg88xx_handle_t *handle) { return ESP_OK; }
esp_err_t amg88xx_set_interrupt_levels(amg88xx_handle_t *handle, float high, float low) { return ESP_OK; }
esp_err_t amg88xx_set_interrupt_levels_manual(amg88xx_handle_t *handle, float high, float low, float hysteresis) { return ESP_OK; }
esp_err_t amg88xx_set_power_mode(amg88xx_handle_t *handle, amg88xx_power_mode_t mode) { return ESP_OK; }
esp_err_t amg88xx_set_frame_rate(amg88xx_handle_t *handle, amg88xx_frame_rate_t rate) { return ESP_OK; }
esp_err_t amg88xx_software_reset(amg88xx_handle_t *handle, amg88xx_sw_reset_t reset_type) { return ESP_OK; }
