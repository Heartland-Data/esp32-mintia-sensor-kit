/**
 * @file xm125_esp32.c
 * @brief XM125 Presence Detector Driver Implementation
 * @copyright Copyright (C) 2026, Heartland Data inc. All Rights Reserved.
 * SPDX-FileCopyrightText: 2026 Heartland Data inc.
 * SPDX-License-Identifier: Apache-2.0
 */

#include "xm125_esp32.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include <string.h>

static const char *TAG = "XM125";

/**
 * @brief XM125センサーを初期化
 * @param handle XM125ハンドル構造体へのポインタ
 * @param config 設定構造体へのポインタ
 * @retval ESP_OK 初期化成功
 * @retval ESP_ERR_INVALID_ARG 引数が不正
 * @retval ESP_ERR_* GPIO設定エラー
 * @details 以下の処理を実行します：
 *          1. 設定をハンドルにコピー
 *          2. ウェイクアップピンを出力モードに設定
 *          3. 割り込みピンを入力モード（プルアップ有効）に設定
 *          4. ウェイクアップピンを初期状態（LOW）に設定
 */
esp_err_t xm125_init(xm125_handle_t *handle, const xm125_config_t *config)
{
    if (handle == NULL || config == NULL) {
        return ESP_ERR_INVALID_ARG;
    }

    // 設定をハンドルにコピー
    memcpy(&handle->config, config, sizeof(xm125_config_t));
    
    // GPIO設定構造体の初期化
    gpio_config_t io_conf = {};
    
    // ウェイクアップピン設定（ESP32 → XM125）
    io_conf.intr_type = GPIO_INTR_DISABLE;               // 割り込み無効
    io_conf.mode = GPIO_MODE_OUTPUT;                     // 出力モード
    io_conf.pin_bit_mask = (1ULL << config->wakeup_pin); // 対象ピン
    io_conf.pull_down_en = GPIO_PULLDOWN_DISABLE;        // プルダウン無効
    io_conf.pull_up_en = GPIO_PULLUP_DISABLE;            // プルアップ無効
    esp_err_t ret = gpio_config(&io_conf);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to configure wakeup pin");
        return ret;
    }
    
    // 割り込みピン設定（XM125 → ESP32）
    io_conf.mode = GPIO_MODE_INPUT;                         // 入力モード
    io_conf.pin_bit_mask = (1ULL << config->interrupt_pin); // 対象ピン
    io_conf.pull_up_en = GPIO_PULLUP_ENABLE;                // プルアップ有効（デフォルトでHIGH）
    ret = gpio_config(&io_conf);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to configure interrupt pin");
        return ret;
    }
    
    // 初期状態でウェイクアップピンをLOWに（センサーはスリープ状態）
    gpio_set_level(config->wakeup_pin, 0);
    
    handle->initialized = true;
    ESP_LOGI(TAG, "XM125 initialized successfully");
    
    return ESP_OK;
}

/**
 * @brief XM125センサーを終了
 * @param handle XM125ハンドル構造体へのポインタ
 * @retval ESP_OK 終了成功
 * @retval ESP_ERR_INVALID_ARG 引数が不正
 * @details 初期化フラグをfalseに設定します。
 *          GPIO設定は保持されます（リセットしない）。
 */
esp_err_t xm125_deinit(xm125_handle_t *handle)
{
    if (handle == NULL) {
        return ESP_ERR_INVALID_ARG;
    }
    
    handle->initialized = false;
    ESP_LOGI(TAG, "XM125 deinitialized");
    
    return ESP_OK;
}

/**
 * @brief XM125モジュールの起動完了を待機
 * @param handle XM125ハンドル構造体へのポインタ
 * @retval ESP_OK 起動完了
 * @retval ESP_ERR_INVALID_ARG 引数が不正または未初期化
 * @retval ESP_ERR_TIMEOUT タイムアウト（3秒）
 * @details 以下の処理を実行します：
 *          1. ウェイクアップピンをHIGHに設定（起動命令）
 *          2. 割り込みピンがHIGHになるまで待機（起動完了の合図）
 *          3. タイムアウト時間は XM125_WAKEUP_TIMEOUT_MS（3000ms）
 * @note 割り込みピンはプルアップされており、通常はHIGH
 *       センサー起動中はLOW、起動完了後にHIGHに戻る
 */
esp_err_t xm125_wait_for_wakeup(xm125_handle_t *handle)
{
    if (handle == NULL || !handle->initialized) {
        return ESP_ERR_INVALID_ARG;
    }
    
    // ウェイクアップピンをHIGHに設定（起動命令送信）
    gpio_set_level(handle->config.wakeup_pin, 1);
    
    // タイムアウト管理用
    TickType_t start_time = xTaskGetTickCount();
    TickType_t timeout_ticks = pdMS_TO_TICKS(XM125_WAKEUP_TIMEOUT_MS);
    
    // 割り込みピンがHIGHになるまで待機（起動完了まで）
    while (gpio_get_level(handle->config.interrupt_pin) == 0) {
        // タイムアウトチェック
        if (xTaskGetTickCount() - start_time > timeout_ticks) {
            ESP_LOGE(TAG, "Wakeup timeout");
            return ESP_ERR_TIMEOUT;
        }
        vTaskDelay(pdMS_TO_TICKS(10)); // 10ms待機
    }
    
    ESP_LOGI(TAG, "Module awake");
    return ESP_OK;
}

/**
 * @brief XM125レジスタから32bitデータを読み取り
 * @param handle XM125ハンドル構造体へのポインタ
 * @param reg_addr レジスタアドレス（16bit）
 * @param reg_data 読み取ったデータを格納するポインタ（32bit）
 * @retval ESP_OK 読み取り成功
 * @retval ESP_ERR_INVALID_ARG 引数が不正または未初期化
 * @retval ESP_ERR_* I2C通信エラー
 * @details I2C通信でレジスタを読み取ります：
 *          1. レジスタアドレス（2byte, Big Endian）を送信
 *          2. データ（4byte, Big Endian）を受信
 *          3. 受信データを32bit整数に変換
 */
esp_err_t xm125_read_register(xm125_handle_t *handle, uint16_t reg_addr, uint32_t *reg_data)
{
    if (handle == NULL || reg_data == NULL || !handle->initialized) {
        return ESP_ERR_INVALID_ARG;
    }
    
    uint8_t write_buf[2] = {
        (uint8_t)(reg_addr >> 8),
        (uint8_t)(reg_addr & 0xFF)
    };
    
    uint8_t read_buf[4];
    
    esp_err_t ret = i2c_master_transmit_receive(
        handle->config.dev_handle,
        write_buf, sizeof(write_buf),
        read_buf, sizeof(read_buf),
        pdMS_TO_TICKS(handle->config.timeout_ms)
    );
    
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "I2C read error at addr 0x%04X: %s", reg_addr, esp_err_to_name(ret));
        return ret;
    }
    
    // Big Endian: MSB first（ビッグエンディアン形式で変換）
    *reg_data = ((uint32_t)read_buf[0] << 24) |
                ((uint32_t)read_buf[1] << 16) |
                ((uint32_t)read_buf[2] << 8) |
                ((uint32_t)read_buf[3]);
    
    return ESP_OK;
}

/**
 * @brief XM125レジスタに32bitデータを書き込み
 * @param handle XM125ハンドル構造体へのポインタ
 * @param reg_addr レジスタアドレス（16bit）
 * @param reg_data 書き込むデータ（32bit）
 * @retval ESP_OK 書き込み成功
 * @retval ESP_ERR_INVALID_ARG 引数が不正または未初期化
 * @retval ESP_ERR_* I2C通信エラー
 * @details I2C通信でレジスタに書き込みます：
 *          1. レジスタアドレス（2byte, Big Endian）
 *          2. データ（4byte, Big Endian）
 *          3. 合計6byteを一度に送信
 */
esp_err_t xm125_write_register(xm125_handle_t *handle, uint16_t reg_addr, uint32_t reg_data)
{
    if (handle == NULL || !handle->initialized) {
        return ESP_ERR_INVALID_ARG;
    }
    
    uint8_t write_buf[6] = {
        (uint8_t)(reg_addr >> 8),
        (uint8_t)(reg_addr & 0xFF),
        (uint8_t)((reg_data >> 24) & 0xFF),
        (uint8_t)((reg_data >> 16) & 0xFF),
        (uint8_t)((reg_data >> 8) & 0xFF),
        (uint8_t)(reg_data & 0xFF)
    };
    
    esp_err_t ret = i2c_master_transmit(
        handle->config.dev_handle,
        write_buf, sizeof(write_buf),
        pdMS_TO_TICKS(handle->config.timeout_ms)
    );
    
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "I2C write error at addr 0x%04X: %s", reg_addr, esp_err_to_name(ret));
    }
    
    return ret;
}

/**
 * @brief デバイスがビジー状態でなくなるまで待機
 * @param handle XM125ハンドル構造体へのポインタ
 * @retval ESP_OK ビジー解除を確認
 * @retval ESP_ERR_INVALID_ARG 引数が不正または未初期化
 * @retval ESP_ERR_TIMEOUT タイムアウト（5秒）
 * @retval ESP_ERR_* I2C通信エラー
 * @details DETECTOR_STATUS レジスタのBUSYビットを監視：
 *          1. DETECTOR_STATUS レジスタ（0x0100）を読み取り
 *          2. BUSYビット（bit 8）が0になるまでポーリング
 *          3. タイムアウトは XM125_BUSY_TIMEOUT_MS（5000ms）
 * @note 設定変更後やコマンド実行後に必ず呼び出す必要があります
 */
esp_err_t xm125_wait_not_busy(xm125_handle_t *handle)
{
    if (handle == NULL || !handle->initialized) {
        return ESP_ERR_INVALID_ARG;
    }
    
    TickType_t start_time = xTaskGetTickCount();
    TickType_t timeout_ticks = pdMS_TO_TICKS(XM125_BUSY_TIMEOUT_MS);
    
    uint32_t status;
    do {
        esp_err_t ret = xm125_read_register(handle, XM125_REG_DETECTOR_STATUS_ADDRESS, &status);
        if (ret != ESP_OK) {
            ESP_LOGE(TAG, "Failed to read detector status");
            return ret;
        }
        
        if (xTaskGetTickCount() - start_time > timeout_ticks) {
            ESP_LOGE(TAG, "Busy timeout (status=0x%08lX)", status);
            return ESP_ERR_TIMEOUT;
        }
        
        if (status & XM125_STATUS_BUSY_MASK) {
            vTaskDelay(pdMS_TO_TICKS(10)); // 10ms待機してから再チェック
        }
        
    } while (status & XM125_STATUS_BUSY_MASK);
    
    return ESP_OK;
}

/**
 * @brief 設定が正常に適用されたか確認
 * @param handle XM125ハンドル構造体へのポインタ
 * @retval true 設定が正常に適用された
 * @retval false 設定適用に失敗または引数エラー
 * @details DETECTOR_STATUS レジスタの6つの必須ビットを確認：
 *          1. RSS_REGISTER_OK（bit 0）: RSSレジスタ読み取り成功
 *          2. CONFIG_CREATE_OK（bit 1）: 設定作成成功
 *          3. SENSOR_CREATE_OK（bit 2）: センサー作成成功
 *          4. SENSOR_CALIBRATE_OK（bit 3）: センサー校正成功
 *          5. DETECTOR_CREATE_OK（bit 4）: ディテクター作成成功
 *          6. DETECTOR_BUFFER_OK（bit 5）: バッファ確保成功
 * @note すべてのビットが1である必要があります
 */
bool xm125_configuration_ok(xm125_handle_t *handle)
{
    if (handle == NULL || !handle->initialized) {
        return false;
    }
    
    uint32_t status;
    esp_err_t ret = xm125_read_register(handle, XM125_REG_DETECTOR_STATUS_ADDRESS, &status);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to read detector status");
        return false;
    }
    
    uint32_t required_bits = XM125_STATUS_RSS_REGISTER_OK_MASK |
                            XM125_STATUS_CONFIG_CREATE_OK_MASK |
                            XM125_STATUS_SENSOR_CREATE_OK_MASK |
                            XM125_STATUS_SENSOR_CALIBRATE_OK_MASK |
                            XM125_STATUS_DETECTOR_CREATE_OK_MASK |
                            XM125_STATUS_DETECTOR_BUFFER_OK_MASK;
    
    if ((status & required_bits) != required_bits) {
        ESP_LOGE(TAG, "Configuration failed. Status: 0x%08lX", status);
        return false;
    }
    
    ESP_LOGI(TAG, "Configuration OK (status=0x%08lX)", status);
    return true;
}

/**
 * @brief 測定設定を適用
 * @param handle XM125ハンドル構造体へのポインタ
 * @param config 測定設定構造体へのポインタ
 * @retval ESP_OK 設定適用成功
 * @retval ESP_ERR_INVALID_ARG 引数が不正または未初期化
 * @retval ESP_FAIL 設定適用後の確認で失敗
 * @retval ESP_ERR_* I2C通信エラー
 * @details 25個のパラメータをレジスタに書き込みます：
 *          1. 検出範囲設定（start_mm, end_mm, max_profile, etc.）
 *          2. 閾値設定（intra/inter detection threshold, etc.）
 *          3. フレーム設定（sweeps_per_frame, frame_rate, etc.）
 *          4. 検出動作設定（detection_on_gpio, etc.）
 *          5. APPLY_CONFIGURATION コマンド実行
 *          6. BUSY解除待機
 *          7. 設定成功確認（6つのOKビット）
 * @note すべての設定は xm125_measurement_config_t 構造体で管理されます
 */
esp_err_t xm125_apply_measurement_config(xm125_handle_t *handle, const xm125_measurement_config_t *config)
{
    if (handle == NULL || config == NULL || !handle->initialized) {
        return ESP_ERR_INVALID_ARG;
    }
    
    ESP_LOGI(TAG, "Applying measurement configuration...");
    
    // 各設定レジスタへ書き込み
    esp_err_t ret;
    
    ret = xm125_write_register(handle, XM125_REG_START_ADDRESS, config->start_mm);
    if (ret != ESP_OK) return ret;
    
    ret = xm125_write_register(handle, XM125_REG_END_ADDRESS, config->end_mm);
    if (ret != ESP_OK) return ret;
    
    ret = xm125_write_register(handle, XM125_REG_INTRA_DETECTION_THRESHOLD_ADDRESS, config->intra_detection_threshold);
    if (ret != ESP_OK) return ret;
    
    ret = xm125_write_register(handle, XM125_REG_INTER_DETECTION_THRESHOLD_ADDRESS, config->inter_detection_threshold);
    if (ret != ESP_OK) return ret;
    
    ret = xm125_write_register(handle, XM125_REG_SWEEPS_PER_FRAME_ADDRESS, config->sweeps_per_frame);
    if (ret != ESP_OK) return ret;
    
    ret = xm125_write_register(handle, XM125_REG_FRAME_RATE_ADDRESS, config->frame_rate);
    if (ret != ESP_OK) return ret;
    
    ret = xm125_write_register(handle, XM125_REG_INTER_FRAME_PRESENCE_TIMEOUT_ADDRESS, config->inter_frame_presence_timeout);
    if (ret != ESP_OK) return ret;
    
    ret = xm125_write_register(handle, XM125_REG_INTRA_DETECTION_ENABLED_ADDRESS, config->intra_detection_enabled);
    if (ret != ESP_OK) return ret;
    
    ret = xm125_write_register(handle, XM125_REG_INTER_DETECTION_ENABLED_ADDRESS, config->inter_detection_enabled);
    if (ret != ESP_OK) return ret;
    
    ret = xm125_write_register(handle, XM125_REG_INTER_PHASE_BOOST_ENABLED_ADDRESS, config->inter_phase_boost_enabled);
    if (ret != ESP_OK) return ret;
    
    ret = xm125_write_register(handle, XM125_REG_INTRA_FRAME_TIME_CONST_ADDRESS, config->intra_frame_time_const);
    if (ret != ESP_OK) return ret;
    
    ret = xm125_write_register(handle, XM125_REG_INTRA_OUTPUT_TIME_CONST_ADDRESS, config->intra_output_time_const);
    if (ret != ESP_OK) return ret;
    
    ret = xm125_write_register(handle, XM125_REG_INTER_OUTPUT_TIME_CONST_ADDRESS, config->inter_output_time_const);
    if (ret != ESP_OK) return ret;
    
    ret = xm125_write_register(handle, XM125_REG_INTER_FRAME_DEVIATION_TIME_CONST_ADDRESS, config->inter_frame_deviation_time_const);
    if (ret != ESP_OK) return ret;
    
    ret = xm125_write_register(handle, XM125_REG_INTER_FRAME_FAST_CUTOFF_ADDRESS, config->inter_frame_fast_cutoff);
    if (ret != ESP_OK) return ret;
    
    ret = xm125_write_register(handle, XM125_REG_INTER_FRAME_SLOW_CUTOFF_ADDRESS, config->inter_frame_slow_cutoff);
    if (ret != ESP_OK) return ret;
    
    ret = xm125_write_register(handle, XM125_REG_AUTO_PROFILE_ENABLED_ADDRESS, config->auto_profile_enabled);
    if (ret != ESP_OK) return ret;
    
    ret = xm125_write_register(handle, XM125_REG_AUTO_STEP_LENGTH_ENABLED_ADDRESS, config->auto_step_length_enabled);
    if (ret != ESP_OK) return ret;
    
    ret = xm125_write_register(handle, XM125_REG_MANUAL_PROFILE_ADDRESS, config->manual_profile);
    if (ret != ESP_OK) return ret;
    
    ret = xm125_write_register(handle, XM125_REG_MANUAL_STEP_LENGTH_ADDRESS, config->manual_step_length);
    if (ret != ESP_OK) return ret;
    
    ret = xm125_write_register(handle, XM125_REG_RESET_FILTERS_ON_PREPARE_ADDRESS, config->reset_filters_on_prepare);
    if (ret != ESP_OK) return ret;
    
    ret = xm125_write_register(handle, XM125_REG_HWAAS_ADDRESS, config->hwaas);
    if (ret != ESP_OK) return ret;
    
    ret = xm125_write_register(handle, XM125_REG_AUTOMATIC_SUBSWEEPS_ADDRESS, config->automatic_subsweeps);
    if (ret != ESP_OK) return ret;
    
    ret = xm125_write_register(handle, XM125_REG_SIGNAL_QUALITY_ADDRESS, config->signal_quality);
    if (ret != ESP_OK) return ret;
    
    ret = xm125_write_register(handle, XM125_REG_DETECTION_ON_GPIO_ADDRESS, config->detection_on_gpio);
    if (ret != ESP_OK) return ret;
    
    // 設定適用コマンド実行（APPLY_CONFIGURATION）
    ret = xm125_write_register(handle, XM125_REG_COMMAND_ADDRESS, XM125_CMD_APPLY_CONFIGURATION);
    if (ret != ESP_OK) return ret;
    
    // ビジー解除待機
    ret = xm125_wait_not_busy(handle);
    if (ret != ESP_OK) return ret;
    
    // 設定成功確認（6つのOKビット）
    if (!xm125_configuration_ok(handle)) {
        return ESP_FAIL;
    }
    
    ESP_LOGI(TAG, "Configuration applied successfully");
    return ESP_OK;
}

/**
 * @brief 検出開始
 * @param handle XM125ハンドル構造体へのポインタ
 * @retval ESP_OK 検出開始成功
 * @retval ESP_ERR_INVALID_ARG 引数が不正または未初期化
 * @retval ESP_ERR_* I2C通信エラー
 * @details 検出を開始します：
 *          1. START_DETECTOR コマンド（0x01）を送信
 *          2. BUSY解除待機
 * @note 検出開始前に設定適用（xm125_apply_measurement_config）が必要です
 */
esp_err_t xm125_start_detector(xm125_handle_t *handle)
{
    if (handle == NULL || !handle->initialized) {
        return ESP_ERR_INVALID_ARG;
    }
    
    ESP_LOGI(TAG, "Starting detector...");
    esp_err_t ret = xm125_write_register(handle, XM125_REG_COMMAND_ADDRESS, XM125_CMD_START_DETECTOR);
    if (ret != ESP_OK) return ret;
    
    ret = xm125_wait_not_busy(handle);
    if (ret != ESP_OK) return ret;
    
    ESP_LOGI(TAG, "Detector started successfully");
    return ESP_OK;
}

/**
 * @brief 検出停止
 * @param handle XM125ハンドル構造体へのポインタ
 * @retval ESP_OK 検出停止成功
 * @retval ESP_ERR_INVALID_ARG 引数が不正または未初期化
 * @retval ESP_ERR_* I2C通信エラー
 * @details 検出を停止します：
 *          1. STOP_DETECTOR コマンド（0x02）を送信
 *          2. BUSY解除待機
 * @note 停止後はスリープモードに移行することが推奨されます
 */
esp_err_t xm125_stop_detector(xm125_handle_t *handle)
{
    if (handle == NULL || !handle->initialized) {
        return ESP_ERR_INVALID_ARG;
    }
    
    ESP_LOGI(TAG, "Stopping detector...");
    esp_err_t ret = xm125_write_register(handle, XM125_REG_COMMAND_ADDRESS, XM125_CMD_STOP_DETECTOR);
    if (ret != ESP_OK) return ret;
    
    ret = xm125_wait_not_busy(handle);
    if (ret != ESP_OK) return ret;
    
    ESP_LOGI(TAG, "Detector stopped successfully");
    return ESP_OK;
}

/**
 * @brief プレゼンス検出結果の読み取り
 * @param handle XM125ハンドル構造体へのポインタ
 * @param result 検出結果を格納する構造体へのポインタ
 * @retval ESP_OK 読み取り成功
 * @retval ESP_ERR_INVALID_ARG 引数が不正または未初期化
 * @retval ESP_ERR_* I2C通信エラー
 * @details プレゼンス検出結果を読み取ります：
 *          1. PRESENCE_RESULT レジスタ（0x0061）を読み取り
 *          2. detected, detected_sticky, detector_error フラグを抽出
 *          3. 両方のdetectedフラグがfalseの場合：
 *             - distance_mm, intra_score, inter_score を -1 に設定
 *             - レジスタ読み取りをスキップ（高速化）
 *          4. 検出ありの場合：
 *             - PRESENCE_DISTANCE（0x0063）から距離を取得
 *             - INTRA_PRESENCE_SCORE（0x0064）からスコアを取得
 *             - INTER_PRESENCE_SCORE（0x0065）からスコアを取得
 * @note -1は「人がいない」を表す特別な値として使用されます
 */
esp_err_t xm125_read_presence_result(xm125_handle_t *handle, xm125_presence_result_t *result)
{
    if (handle == NULL || result == NULL || !handle->initialized) {
        return ESP_ERR_INVALID_ARG;
    }
    
    uint32_t presence_result;
    esp_err_t ret = xm125_read_register(handle, XM125_REG_PRESENCE_RESULT_ADDRESS, &presence_result);
    if (ret != ESP_OK) return ret;
    
    result->presence_detected = (presence_result & XM125_PRESENCE_DETECTED_MASK) != 0;
    result->presence_detected_sticky = (presence_result & XM125_PRESENCE_DETECTED_STICKY_MASK) != 0;
    result->detector_error = (presence_result & XM125_DETECTOR_ERROR_MASK) != 0;
    
    // 存在検知がない場合は距離とスコアを-1に設定（レジスタ読まない）
    if (!result->presence_detected && !result->presence_detected_sticky) {
        result->distance_mm = -1;
        result->intra_score = -1;
        result->inter_score = -1;
        return ESP_OK;
    }
    
    // 存在検知がある場合のみレジスタ読み取り
    uint32_t distance, intra, inter;
    ret = xm125_read_register(handle, XM125_REG_PRESENCE_DISTANCE_ADDRESS, &distance);
    if (ret != ESP_OK) return ret;
    
    ret = xm125_read_register(handle, XM125_REG_INTRA_PRESENCE_SCORE_ADDRESS, &intra);
    if (ret != ESP_OK) return ret;
    
    ret = xm125_read_register(handle, XM125_REG_INTER_PRESENCE_SCORE_ADDRESS, &inter);
    if (ret != ESP_OK) return ret;
    
    result->distance_mm = (int32_t)distance;
    result->intra_score = (int32_t)intra;
    result->inter_score = (int32_t)inter;
    
    return ESP_OK;
}
