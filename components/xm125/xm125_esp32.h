/**
 * @file xm125_esp32.h
 * @brief XM125 Presence Detector Driver for ESP32
 * @copyright Copyright (C) 2026, Heartland Data inc. All Rights Reserved.
 * SPDX-FileCopyrightText: 2026 Heartland Data inc.
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include "esp_err.h"
#include "driver/i2c_master.h"
#include "driver/gpio.h"
#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

// XM125のデフォルト設定
#define XM125_DEFAULT_I2C_ADDR 0x52
#define XM125_REG_ADDRESS_LENGTH 2
#define XM125_REG_DATA_LENGTH 4

// タイムアウト設定
#define XM125_I2C_TIMEOUT_MS 1000
#define XM125_BUSY_TIMEOUT_MS 5000
#define XM125_WAKEUP_TIMEOUT_MS 3000

// XM125 レジスタアドレス
#define XM125_REG_VERSION_ADDRESS 0x0000
#define XM125_REG_DETECTOR_STATUS_ADDRESS 0x0003
#define XM125_REG_PRESENCE_RESULT_ADDRESS 0x0010
#define XM125_REG_PRESENCE_DISTANCE_ADDRESS 0x0011
#define XM125_REG_INTRA_PRESENCE_SCORE_ADDRESS 0x0012
#define XM125_REG_INTER_PRESENCE_SCORE_ADDRESS 0x0013
#define XM125_REG_SWEEPS_PER_FRAME_ADDRESS 0x0040
#define XM125_REG_INTER_FRAME_PRESENCE_TIMEOUT_ADDRESS 0x0041
#define XM125_REG_INTER_PHASE_BOOST_ENABLED_ADDRESS 0x0042
#define XM125_REG_INTRA_DETECTION_ENABLED_ADDRESS 0x0043
#define XM125_REG_INTER_DETECTION_ENABLED_ADDRESS 0x0044
#define XM125_REG_FRAME_RATE_ADDRESS 0x0045
#define XM125_REG_INTRA_DETECTION_THRESHOLD_ADDRESS 0x0046
#define XM125_REG_INTER_DETECTION_THRESHOLD_ADDRESS 0x0047
#define XM125_REG_INTER_FRAME_DEVIATION_TIME_CONST_ADDRESS 0x0048
#define XM125_REG_INTER_FRAME_FAST_CUTOFF_ADDRESS 0x0049
#define XM125_REG_INTER_FRAME_SLOW_CUTOFF_ADDRESS 0x004A
#define XM125_REG_INTRA_FRAME_TIME_CONST_ADDRESS 0x004B
#define XM125_REG_INTRA_OUTPUT_TIME_CONST_ADDRESS 0x004C
#define XM125_REG_INTER_OUTPUT_TIME_CONST_ADDRESS 0x004D
#define XM125_REG_AUTO_PROFILE_ENABLED_ADDRESS 0x004E
#define XM125_REG_AUTO_STEP_LENGTH_ENABLED_ADDRESS 0x004F
#define XM125_REG_MANUAL_PROFILE_ADDRESS 0x0050
#define XM125_REG_MANUAL_STEP_LENGTH_ADDRESS 0x0051
#define XM125_REG_START_ADDRESS 0x0052
#define XM125_REG_END_ADDRESS 0x0053
#define XM125_REG_RESET_FILTERS_ON_PREPARE_ADDRESS 0x0054
#define XM125_REG_HWAAS_ADDRESS 0x0055
#define XM125_REG_AUTOMATIC_SUBSWEEPS_ADDRESS 0x0056
#define XM125_REG_SIGNAL_QUALITY_ADDRESS 0x0057
#define XM125_REG_DETECTION_ON_GPIO_ADDRESS 0x0080
#define XM125_REG_COMMAND_ADDRESS 0x0100

// ステータスビットマスク
#define XM125_STATUS_RSS_REGISTER_OK_MASK 0x00000001
#define XM125_STATUS_CONFIG_CREATE_OK_MASK 0x00000002
#define XM125_STATUS_SENSOR_CREATE_OK_MASK 0x00000004
#define XM125_STATUS_SENSOR_CALIBRATE_OK_MASK 0x00000008
#define XM125_STATUS_DETECTOR_CREATE_OK_MASK 0x00000010
#define XM125_STATUS_DETECTOR_BUFFER_OK_MASK 0x00000020
#define XM125_STATUS_CALIBRATION_STATE_MASK 0x00000300
#define XM125_STATUS_BUSY_MASK 0x80000000

// Presence Result ビットマスク
#define XM125_PRESENCE_DETECTED_MASK 0x00000001
#define XM125_PRESENCE_DETECTED_STICKY_MASK 0x00000002
#define XM125_DETECTOR_ERROR_MASK 0x00008000

// コマンド定義
#define XM125_CMD_APPLY_CONFIGURATION 0x00000001
#define XM125_CMD_START_DETECTOR 0x00000002
#define XM125_CMD_STOP_DETECTOR 0x00000003
#define XM125_CMD_ENABLE_UART_LOGS 0x00000004
#define XM125_CMD_DISABLE_UART_LOGS 0x00000005
#define XM125_CMD_LOG_CONFIGURATION 0x00000006
#define XM125_CMD_RESET_MODULE 0x00000007
#define XM125_CMD_APPLY_CONFIGURATION_AND_CALIBRATE 0x00000008
#define XM125_CMD_RECALIBRATE 0x00000009

// デフォルト設定値（全25パラメータ）
#define XM125_DEFAULT_SWEEPS_PER_FRAME 16                       // 1フレームあたりのスイープ数
#define XM125_DEFAULT_INTER_FRAME_PRESENCE_TIMEOUT 3            // フレーム間プレゼンスタイムアウト（秒）
#define XM125_DEFAULT_INTER_PHASE_BOOST_ENABLED 0               // 位相ブースト有効（False）
#define XM125_DEFAULT_INTRA_DETECTION_ENABLED 1                 // フレーム内検出有効（True）
#define XM125_DEFAULT_INTER_DETECTION_ENABLED 1                 // フレーム間検出有効（True）
#define XM125_DEFAULT_FRAME_RATE 12000                          // フレームレート（mHz）
#define XM125_DEFAULT_INTRA_DETECTION_THRESHOLD 1300            // フレーム内検出閾値
#define XM125_DEFAULT_INTER_DETECTION_THRESHOLD 1000            // フレーム間検出閾値
#define XM125_DEFAULT_INTER_FRAME_DEVIATION_TIME_CONST 500      // フレーム間偏差時定数（ms）
#define XM125_DEFAULT_INTER_FRAME_FAST_CUTOFF 6000              // フレーム間高速カットオフ（mHz）
#define XM125_DEFAULT_INTER_FRAME_SLOW_CUTOFF 200               // フレーム間低速カットオフ（mHz）
#define XM125_DEFAULT_INTRA_FRAME_TIME_CONST 150                // フレーム内時定数（ms）
#define XM125_DEFAULT_INTRA_OUTPUT_TIME_CONST 300               // フレーム内出力時定数（ms）
#define XM125_DEFAULT_INTER_OUTPUT_TIME_CONST 2000              // フレーム間出力時定数（ms）
#define XM125_DEFAULT_AUTO_PROFILE_ENABLED 1                    // 自動プロファイル有効（True）
#define XM125_DEFAULT_AUTO_STEP_LENGTH_ENABLED 1                // 自動ステップ長有効（True）
#define XM125_DEFAULT_MANUAL_PROFILE 4                          // 手動プロファイル（PROFILE4）
#define XM125_DEFAULT_MANUAL_STEP_LENGTH 72                     // 手動ステップ長
#define XM125_DEFAULT_START_MM 300                              // 測定開始点（mm）
#define XM125_DEFAULT_END_MM 2500                               // 測定終了点（mm）
#define XM125_DEFAULT_RESET_FILTERS_ON_PREPARE 1                // フィルタリセット有効（True）
#define XM125_DEFAULT_HWAAS 32                                  // ハードウェア加速平均サンプル
#define XM125_DEFAULT_AUTOMATIC_SUBSWEEPS 1                     // 自動サブスイープ（True）
#define XM125_DEFAULT_SIGNAL_QUALITY 15000                      // 信号品質
#define XM125_DEFAULT_DETECTION_ON_GPIO 0                       // GPIO検出出力（False）


/**
 * @brief デフォルト測定設定初期化マクロ
 */
#define XM125_DEFAULT_MEASUREMENT_CONFIG() \
    { \
        .start_mm = XM125_DEFAULT_START_MM, \
        .end_mm = XM125_DEFAULT_END_MM, \
        .intra_detection_threshold = XM125_DEFAULT_INTRA_DETECTION_THRESHOLD, \
        .inter_detection_threshold = XM125_DEFAULT_INTER_DETECTION_THRESHOLD, \
        .sweeps_per_frame = XM125_DEFAULT_SWEEPS_PER_FRAME, \
        .frame_rate = XM125_DEFAULT_FRAME_RATE, \
        .inter_frame_presence_timeout = XM125_DEFAULT_INTER_FRAME_PRESENCE_TIMEOUT, \
        .intra_detection_enabled = XM125_DEFAULT_INTRA_DETECTION_ENABLED, \
        .inter_detection_enabled = XM125_DEFAULT_INTER_DETECTION_ENABLED, \
        .inter_phase_boost_enabled = XM125_DEFAULT_INTER_PHASE_BOOST_ENABLED, \
        .intra_frame_time_const = XM125_DEFAULT_INTRA_FRAME_TIME_CONST, \
        .intra_output_time_const = XM125_DEFAULT_INTRA_OUTPUT_TIME_CONST, \
        .inter_output_time_const = XM125_DEFAULT_INTER_OUTPUT_TIME_CONST, \
        .inter_frame_deviation_time_const = XM125_DEFAULT_INTER_FRAME_DEVIATION_TIME_CONST, \
        .inter_frame_fast_cutoff = XM125_DEFAULT_INTER_FRAME_FAST_CUTOFF, \
        .inter_frame_slow_cutoff = XM125_DEFAULT_INTER_FRAME_SLOW_CUTOFF, \
        .auto_profile_enabled = XM125_DEFAULT_AUTO_PROFILE_ENABLED, \
        .auto_step_length_enabled = XM125_DEFAULT_AUTO_STEP_LENGTH_ENABLED, \
        .manual_profile = XM125_DEFAULT_MANUAL_PROFILE, \
        .manual_step_length = XM125_DEFAULT_MANUAL_STEP_LENGTH, \
        .reset_filters_on_prepare = XM125_DEFAULT_RESET_FILTERS_ON_PREPARE, \
        .hwaas = XM125_DEFAULT_HWAAS, \
        .automatic_subsweeps = XM125_DEFAULT_AUTOMATIC_SUBSWEEPS, \
        .signal_quality = XM125_DEFAULT_SIGNAL_QUALITY, \
        .detection_on_gpio = XM125_DEFAULT_DETECTION_ON_GPIO \
    }

/**
 * @brief XM125設定構造体
 */
typedef struct {
    i2c_master_dev_handle_t dev_handle;
    int timeout_ms;
    gpio_num_t wakeup_pin;
    gpio_num_t interrupt_pin;
} xm125_config_t;

/**
 * @brief XM125ハンドル構造体
 */
typedef struct {
    xm125_config_t config;
    bool initialized;
} xm125_handle_t;

/**
 * @brief XM125測定設定構造体
 */
typedef struct {
    uint32_t start_mm;
    uint32_t end_mm;
    uint32_t intra_detection_threshold;
    uint32_t inter_detection_threshold;
    uint32_t sweeps_per_frame;
    uint32_t frame_rate;
    uint32_t inter_frame_presence_timeout;
    uint32_t intra_detection_enabled;
    uint32_t inter_detection_enabled;
    uint32_t inter_phase_boost_enabled;
    uint32_t intra_frame_time_const;
    uint32_t intra_output_time_const;
    uint32_t inter_output_time_const;
    uint32_t inter_frame_deviation_time_const;
    uint32_t inter_frame_fast_cutoff;
    uint32_t inter_frame_slow_cutoff;
    uint32_t auto_profile_enabled;
    uint32_t auto_step_length_enabled;
    uint32_t manual_profile;
    uint32_t manual_step_length;
    uint32_t reset_filters_on_prepare;
    uint32_t hwaas;
    uint32_t automatic_subsweeps;
    uint32_t signal_quality;
    uint32_t detection_on_gpio;
} xm125_measurement_config_t;

/**
 * @brief XM125プレゼンス結果構造体
 */
typedef struct {
    bool presence_detected;
    bool presence_detected_sticky;
    bool detector_error;
    int32_t distance_mm;    // -1 = 存在なし
    int32_t intra_score;    // -1 = 存在なし
    int32_t inter_score;    // -1 = 存在なし
} xm125_presence_result_t;

// 関数プロトタイプ
esp_err_t xm125_init(xm125_handle_t *handle, const xm125_config_t *config);
esp_err_t xm125_deinit(xm125_handle_t *handle);
esp_err_t xm125_wait_for_wakeup(xm125_handle_t *handle);
esp_err_t xm125_read_register(xm125_handle_t *handle, uint16_t reg_addr, uint32_t *reg_data);
esp_err_t xm125_write_register(xm125_handle_t *handle, uint16_t reg_addr, uint32_t reg_data);
esp_err_t xm125_wait_not_busy(xm125_handle_t *handle);
esp_err_t xm125_apply_measurement_config(xm125_handle_t *handle, const xm125_measurement_config_t *config);
esp_err_t xm125_start_detector(xm125_handle_t *handle);
esp_err_t xm125_stop_detector(xm125_handle_t *handle);
esp_err_t xm125_read_presence_result(xm125_handle_t *handle, xm125_presence_result_t *result);
bool xm125_configuration_ok(xm125_handle_t *handle);

#ifdef __cplusplus
}
#endif
