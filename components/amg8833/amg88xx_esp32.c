/**
 * @file amg88xx_esp32.c
 * @brief AMG88xx 赤外線アレイセンサ ESP32用ドライバ 実装ファイル
 *
 * 本ファイルは、AMG88xx（8x8赤外線アレイセンサ）をESP-IDF環境で制御するための
 * C言語によるドライバ実装です。I2C通信を用いてセンサの初期化、温度データ取得、
 * 割り込み制御などの機能を提供します。
 *
 * @copyright
 * MIT License
 *
 * Original work:
 *   Copyright (c) Adafruit Industries
 *   Written by Dean Miller for Adafruit Industries
 *   https://github.com/adafruit/Adafruit_AMG88xx
 *
 * This file is an ESP-IDF port of the original Adafruit AMG88xx Arduino
 * library. The original MIT License terms apply; see LICENSE for full text.
 * SPDX-FileCopyrightText: Adafruit Industries
 * SPDX-License-Identifier: MIT
 */

#include "amg88xx_esp32.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include <string.h>
#include <math.h>

static const char *TAG = "AMG88XX";

// Helper macros for constraining values
#define CONSTRAIN(x, a, b) ((x) < (a) ? (a) : ((x) > (b) ? (b) : (x)))
#define MIN(a, b) ((a) < (b) ? (a) : (b))

// Internal function prototypes
static esp_err_t amg88xx_write_register(amg88xx_handle_t *handle, uint8_t reg, uint8_t value);
static esp_err_t amg88xx_read_register(amg88xx_handle_t *handle, uint8_t reg, uint8_t *value);
static esp_err_t amg88xx_read_registers(amg88xx_handle_t *handle, uint8_t reg, uint8_t *buf, size_t len);
static float signed_mag12_to_float(uint16_t val);
static float int12_to_float(uint16_t val);

/**
 * @brief AMG88xxセンサを初期化します
 *
 * @param handle AMG88xxハンドルへのポインタ
 * @param config 設定構造体へのポインタ
 * @retval ESP_OK 正常終了
 */
esp_err_t amg88xx_init(amg88xx_handle_t *handle, const amg88xx_config_t *config)
{
    if (handle == NULL || config == NULL)
    {
        return ESP_ERR_INVALID_ARG;
    }

    // Copy configuration
    memcpy(&handle->config, config, sizeof(amg88xx_config_t));

    // Test I2C communication
    uint8_t test_val;
    esp_err_t ret = amg88xx_read_register(handle, AMG88XX_PCTL, &test_val);
    if (ret != ESP_OK)
    {
        ESP_LOGE(TAG, "Failed to communicate with AMG88xx sensor");
        return ret;
    }

    // Enter normal mode
    ret = amg88xx_set_power_mode(handle, AMG88XX_NORMAL_MODE);
    if (ret != ESP_OK)
    {
        ESP_LOGE(TAG, "Failed to set normal mode");
        return ret;
    }

    // Software reset
    ret = amg88xx_software_reset(handle, AMG88XX_INITIAL_RESET);
    if (ret != ESP_OK)
    {
        ESP_LOGE(TAG, "Failed to perform initial reset");
        return ret;
    }

    // Disable interrupts by default
    ret = amg88xx_disable_interrupt(handle);
    if (ret != ESP_OK)
    {
        ESP_LOGE(TAG, "Failed to disable interrupts");
        return ret;
    }

    // Set to 10 FPS
    ret = amg88xx_set_frame_rate(handle, AMG88XX_FPS_10);
    if (ret != ESP_OK)
    {
        ESP_LOGE(TAG, "Failed to set frame rate");
        return ret;
    }

    // Wait for initialization to complete
    vTaskDelay(pdMS_TO_TICKS(100));

    handle->initialized = true;
    ESP_LOGI(TAG, "AMG88xx sensor initialized successfully");

    return ESP_OK;
}

/**
 * @brief AMG88xxセンサの初期化を解除します
 *
 * @param handle AMG88xxハンドルへのポインタ
 * @retval ESP_OK 正常終了
 */
esp_err_t amg88xx_deinit(amg88xx_handle_t *handle)
{
    if (handle == NULL)
    {
        return ESP_ERR_INVALID_ARG;
    }

    handle->initialized = false;
    ESP_LOGI(TAG, "AMG88xx sensor deinitialized");

    return ESP_OK;
}

/**
 * @brief ピクセル温度データを生データ（バイト配列）で読み取ります
 *
 * @param handle AMG88xxハンドルへのポインタ
 * @param buf 64ピクセル分（128バイト）の生データ格納用バッファ
 * @param pixels 読み取るピクセル数（最大64）
 * @retval ESP_OK 正常終了
 */
esp_err_t amg88xx_read_pixels_raw(amg88xx_handle_t *handle, uint8_t *buf, uint8_t pixels)
{
    if (handle == NULL || buf == NULL || !handle->initialized)
    {
        return ESP_ERR_INVALID_ARG;
    }

    uint8_t bytes_to_read = MIN(pixels << 1, AMG88XX_PIXEL_ARRAY_SIZE << 1);
    return amg88xx_read_registers(handle, AMG88XX_PIXEL_OFFSET, buf, bytes_to_read);
}

/**
 * @brief ピクセル温度データを摂氏のfloat値で読み取ります
 *
 * @param handle AMG88xxハンドルへのポインタ
 * @param buf 温度値（摂氏）格納用バッファ
 * @param pixels 読み取るピクセル数（最大64）
 * @retval ESP_OK 正常終了
 */
esp_err_t amg88xx_read_pixels(amg88xx_handle_t *handle, float *buf, uint8_t pixels)
{
    if (handle == NULL || buf == NULL || !handle->initialized)
    {
        return ESP_ERR_INVALID_ARG;
    }

    uint8_t bytes_to_read = MIN(pixels << 1, AMG88XX_PIXEL_ARRAY_SIZE << 1);
    uint8_t raw_array[bytes_to_read];

    esp_err_t ret = amg88xx_read_registers(handle, AMG88XX_PIXEL_OFFSET, raw_array, bytes_to_read);
    if (ret != ESP_OK)
    {
        return ret;
    }

    for (int i = 0; i < pixels; i++)
    {
        uint8_t pos = i << 1;
        uint16_t recast = ((uint16_t)raw_array[pos + 1] << 8) | ((uint16_t)raw_array[pos]);
        buf[i] = int12_to_float(recast) * AMG88XX_PIXEL_TEMP_CONVERSION;
    }

    return ESP_OK;
}

/**
 * @brief サーミスタ温度を読み取ります
 *
 * @param handle AMG88xxハンドルへのポインタ
 * @param temperature 温度値（摂氏）格納用ポインタ
 * @retval ESP_OK 正常終了
 */
esp_err_t amg88xx_read_thermistor(amg88xx_handle_t *handle, float *temperature)
{
    if (handle == NULL || temperature == NULL || !handle->initialized)
    {
        return ESP_ERR_INVALID_ARG;
    }

    uint8_t raw[2];
    esp_err_t ret = amg88xx_read_registers(handle, AMG88XX_TTHL, raw, 2);
    if (ret != ESP_OK)
    {
        return ret;
    }

    uint16_t recast = ((uint16_t)raw[1] << 8) | ((uint16_t)raw[0]);
    *temperature = signed_mag12_to_float(recast) * AMG88XX_THERMISTOR_CONVERSION;

    return ESP_OK;
}

/**
 * @brief 移動平均モードを設定します
 *
 * @param handle AMG88xxハンドルへのポインタ
 * @param enable trueで2倍移動平均モード有効
 * @retval ESP_OK 正常終了
 */
esp_err_t amg88xx_set_moving_average_mode(amg88xx_handle_t *handle, bool enable)
{
    if (handle == NULL || !handle->initialized)
    {
        return ESP_ERR_INVALID_ARG;
    }

    uint8_t value = enable ? 0x20 : 0x00; // MAMOD bit is bit 5
    return amg88xx_write_register(handle, AMG88XX_AVE, value);
}

/**
 * @brief 割り込みを有効化します
 *
 * @param handle AMG88xxハンドルへのポインタ
 * @retval ESP_OK 正常終了
 */
esp_err_t amg88xx_enable_interrupt(amg88xx_handle_t *handle)
{
    if (handle == NULL || !handle->initialized)
    {
        return ESP_ERR_INVALID_ARG;
    }

    uint8_t current_value;
    esp_err_t ret = amg88xx_read_register(handle, AMG88XX_INTC, &current_value);
    if (ret != ESP_OK)
    {
        return ret;
    }

    current_value |= 0x01; // Set INTEN bit
    return amg88xx_write_register(handle, AMG88XX_INTC, current_value);
}

/**
 * @brief 割り込みを無効化します
 *
 * @param handle AMG88xxハンドルへのポインタ
 * @retval ESP_OK 正常終了
 */
esp_err_t amg88xx_disable_interrupt(amg88xx_handle_t *handle)
{
    if (handle == NULL || !handle->initialized)
    {
        return ESP_ERR_INVALID_ARG;
    }

    uint8_t current_value;
    esp_err_t ret = amg88xx_read_register(handle, AMG88XX_INTC, &current_value);
    if (ret != ESP_OK)
    {
        return ret;
    }

    current_value &= ~0x01; // Clear INTEN bit
    return amg88xx_write_register(handle, AMG88XX_INTC, current_value);
}

/**
 * @brief 割り込みモードを設定します
 *
 * @param handle AMG88xxハンドルへのポインタ
 * @param mode 割り込みモード（差分または絶対値）
 * @retval ESP_OK 正常終了
 */
esp_err_t amg88xx_set_interrupt_mode(amg88xx_handle_t *handle, amg88xx_int_mode_t mode)
{
    if (handle == NULL || !handle->initialized)
    {
        return ESP_ERR_INVALID_ARG;
    }

    uint8_t current_value;
    esp_err_t ret = amg88xx_read_register(handle, AMG88XX_INTC, &current_value);
    if (ret != ESP_OK)
    {
        return ret;
    }

    if (mode == AMG88XX_ABSOLUTE_VALUE)
    {
        current_value |= 0x02; // Set INTMOD bit
    }
    else
    {
        current_value &= ~0x02; // Clear INTMOD bit
    }

    return amg88xx_write_register(handle, AMG88XX_INTC, current_value);
}

/**
 * @brief 割り込みステータスを取得します
 *
 * @param handle AMG88xxハンドルへのポインタ
 * @param buf 割り込みデータ格納用バッファ（8バイト）
 * @param size 読み取るバイト数（最大8）
 * @retval ESP_OK 正常終了
 */
esp_err_t amg88xx_get_interrupt(amg88xx_handle_t *handle, uint8_t *buf, uint8_t size)
{
    if (handle == NULL || buf == NULL || !handle->initialized)
    {
        return ESP_ERR_INVALID_ARG;
    }

    uint8_t bytes_to_read = MIN(size, 8);
    return amg88xx_read_registers(handle, AMG88XX_INT_OFFSET, buf, bytes_to_read);
}

/**
 * @brief 割り込みフラグをクリアします
 *
 * @param handle AMG88xxハンドルへのポインタ
 * @retval ESP_OK 正常終了
 */
esp_err_t amg88xx_clear_interrupt(amg88xx_handle_t *handle)
{
    if (handle == NULL || !handle->initialized)
    {
        return ESP_ERR_INVALID_ARG;
    }

    return amg88xx_software_reset(handle, AMG88XX_FLAG_RESET);
}

/**
 * @brief 自動ヒステリシスで割り込みレベルを設定します
 *
 * @param handle AMG88xxハンドルへのポインタ
 * @param high 高しきい値温度
 * @param low 低しきい値温度
 * @retval ESP_OK 正常終了
 */
esp_err_t amg88xx_set_interrupt_levels(amg88xx_handle_t *handle, float high, float low)
{
    return amg88xx_set_interrupt_levels_manual(handle, high, low, high * 0.95f);
}

/**
 * @brief 手動ヒステリシスで割り込みレベルを設定します
 *
 * @param handle AMG88xxハンドルへのポインタ
 * @param high 高しきい値温度
 * @param low 低しきい値温度
 * @param hysteresis ヒステリシス値
 * @retval ESP_OK 正常終了
 */
esp_err_t amg88xx_set_interrupt_levels_manual(amg88xx_handle_t *handle, float high, float low, float hysteresis)
{
    if (handle == NULL || !handle->initialized)
    {
        return ESP_ERR_INVALID_ARG;
    }

    esp_err_t ret;

    // Set high threshold
    int high_conv = (int)(high / AMG88XX_PIXEL_TEMP_CONVERSION);
    high_conv = CONSTRAIN(high_conv, -4095, 4095);

    ret = amg88xx_write_register(handle, AMG88XX_INTHL, high_conv & 0xFF);
    if (ret != ESP_OK)
        return ret;

    ret = amg88xx_write_register(handle, AMG88XX_INTHH, (high_conv & 0x0F00) >> 8);
    if (ret != ESP_OK)
        return ret;

    // Set low threshold
    int low_conv = (int)(low / AMG88XX_PIXEL_TEMP_CONVERSION);
    low_conv = CONSTRAIN(low_conv, -4095, 4095);

    ret = amg88xx_write_register(handle, AMG88XX_INTLL, low_conv & 0xFF);
    if (ret != ESP_OK)
        return ret;

    ret = amg88xx_write_register(handle, AMG88XX_INTLH, (low_conv & 0x0F00) >> 8);
    if (ret != ESP_OK)
        return ret;

    // Set hysteresis
    int hys_conv = (int)(hysteresis / AMG88XX_PIXEL_TEMP_CONVERSION);
    hys_conv = CONSTRAIN(hys_conv, -4095, 4095);

    ret = amg88xx_write_register(handle, AMG88XX_IHYSL, hys_conv & 0xFF);
    if (ret != ESP_OK)
        return ret;

    ret = amg88xx_write_register(handle, AMG88XX_IHYSH, (hys_conv & 0x0F00) >> 8);
    if (ret != ESP_OK)
        return ret;

    return ESP_OK;
}

/**
 * @brief 電源モードを設定します
 *
 * @param handle AMG88xxハンドルへのポインタ
 * @param mode 電源モード
 * @retval ESP_OK 正常終了
 */
esp_err_t amg88xx_set_power_mode(amg88xx_handle_t *handle, amg88xx_power_mode_t mode)
{
    if (handle == NULL || !handle->initialized)
    {
        return ESP_ERR_INVALID_ARG;
    }

    return amg88xx_write_register(handle, AMG88XX_PCTL, (uint8_t)mode);
}

/**
 * @brief フレームレートを設定します
 *
 * @param handle AMG88xxハンドルへのポインタ
 * @param rate フレームレート（1FPSまたは10FPS）
 * @retval ESP_OK 正常終了
 */
esp_err_t amg88xx_set_frame_rate(amg88xx_handle_t *handle, amg88xx_frame_rate_t rate)
{
    if (handle == NULL || !handle->initialized)
    {
        return ESP_ERR_INVALID_ARG;
    }

    return amg88xx_write_register(handle, AMG88XX_FPSC, (uint8_t)rate);
}

/**
 * @brief ソフトウェアリセットを実行します
 *
 * @param handle AMG88xxハンドルへのポインタ
 * @param reset_type リセット種別
 * @retval ESP_OK 正常終了
 */
esp_err_t amg88xx_software_reset(amg88xx_handle_t *handle, amg88xx_sw_reset_t reset_type)
{
    if (handle == NULL)
    {
        return ESP_ERR_INVALID_ARG;
    }

    return amg88xx_write_register(handle, AMG88XX_RST, (uint8_t)reset_type);
}

// Internal helper functions

static esp_err_t amg88xx_write_register(amg88xx_handle_t *handle, uint8_t reg, uint8_t value)
{
    uint8_t write_buf[2] = {reg, value};

    esp_err_t ret = i2c_master_transmit(
        handle->config.dev_handle,
        write_buf,
        2,
        pdMS_TO_TICKS(handle->config.timeout_ms));

    if (ret != ESP_OK)
    {
        ESP_LOGE(TAG, "Failed to write register 0x%02X: %s", reg, esp_err_to_name(ret));
    }

    return ret;
}

static esp_err_t amg88xx_read_register(amg88xx_handle_t *handle, uint8_t reg, uint8_t *value)
{
    esp_err_t ret = i2c_master_transmit_receive(
        handle->config.dev_handle,
        &reg,
        1,
        value,
        1,
        pdMS_TO_TICKS(handle->config.timeout_ms));

    if (ret != ESP_OK)
    {
        ESP_LOGE(TAG, "Failed to read register 0x%02X: %s", reg, esp_err_to_name(ret));
    }

    return ret;
}

static esp_err_t amg88xx_read_registers(amg88xx_handle_t *handle, uint8_t reg, uint8_t *buf, size_t len)
{
    esp_err_t ret = i2c_master_transmit_receive(
        handle->config.dev_handle,
        &reg,
        1,
        buf,
        len,
        pdMS_TO_TICKS(handle->config.timeout_ms));

    if (ret != ESP_OK)
    {
        ESP_LOGE(TAG, "Failed to read %d bytes from register 0x%02X: %s", len, reg, esp_err_to_name(ret));
    }

    return ret;
}

static float signed_mag12_to_float(uint16_t val)
{
    // Take first 11 bits as absolute value
    uint16_t abs_val = (val & 0x7FF);

    // Check sign bit (bit 11)
    return (val & 0x800) ? -(float)abs_val : (float)abs_val;
}

static float int12_to_float(uint16_t val)
{
    // Shift to left so that sign bit of 12 bit integer number is
    // placed on sign bit of 16 bit signed integer number
    int16_t s_val = (val << 4);

    // Shift back the signed number, return converts to float
    return (float)(s_val >> 4);
}
