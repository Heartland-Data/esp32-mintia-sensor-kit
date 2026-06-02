/**
 * @file xm125_sensor_service.cpp
 * @brief XM125（存在検知）用センササービス実装
 * @copyright Copyright (C) 2026, Heartland Data inc. All Rights Reserved.
 * SPDX-FileCopyrightText: 2026 Heartland Data inc.
 * SPDX-License-Identifier: Apache-2.0
 */

#include "xm125_sensor_service.hpp"
#include "esp_log.h"
#include <cstdio>
#include <vector>
#include "command_interface/communication_packet.hpp"
#include "command_interface/sensor_data_message.hpp"

namespace service
{
    namespace
    {
        // XM125測定設定カスタマイズ値
        // 変更したいパラメータだけコメントを外して値を編集する
        // #define kCustomSweepsPerFrame 16                                      // 1フレームあたりのスイープ数
        // #define kCustomInterFramePresenceTimeout 3                            // フレーム間プレゼンスタイムアウト（秒）
        // #define kCustomInterPhaseBoostEnabled 0                               // 位相ブースト有効（1=有効, 0=無効）
        // #define kCustomIntraDetectionEnabled 1                                // フレーム内検出有効（1=有効, 0=無効）
        // #define kCustomInterDetectionEnabled 1                                // フレーム間検出有効（1=有効, 0=無効）
        // #define kCustomFrameRate 12000                                        // フレームレート（mHz）
        // #define kCustomIntraDetectionThreshold 1300                           // フレーム内検出閾値
        // #define kCustomInterDetectionThreshold 1000                           // フレーム間検出閾値
        // #define kCustomInterFrameDeviationTimeConst 500                       // フレーム間偏差時定数（ms）
        // #define kCustomInterFrameFastCutoff 6000                              // フレーム間高速カットオフ（mHz）
        // #define kCustomInterFrameSlowCutoff 200                               // フレーム間低速カットオフ（mHz）
        // #define kCustomIntraFrameTimeConst 150                                // フレーム内時定数（ms）
        // #define kCustomIntraOutputTimeConst 300                               // フレーム内出力時定数（ms）
        // #define kCustomInterOutputTimeConst 2000                              // フレーム間出力時定数（ms）
        // #define kCustomAutoProfileEnabled 1                                   // 自動プロファイル有効（1=有効, 0=無効）
        // #define kCustomAutoStepLengthEnabled 1                                // 自動ステップ長有効（1=有効, 0=無効）
        // #define kCustomManualProfile 4                                        // 手動プロファイル（1-5: PROFILE1-5）
        // #define kCustomManualStepLength 72                                    // 手動ステップ長
        // #define kCustomStartMm 300                                            // 測定開始点（mm）
        // #define kCustomEndMm 2500                                             // 測定終了点（mm）
        // #define kCustomResetFiltersOnPrepare 1                                // フィルタリセット有効（1=有効, 0=無効）
        // #define kCustomHwaas 32                                               // ハードウェア加速平均サンプル
        // #define kCustomAutomaticSubsweeps 1                                   // 自動サブスイープ（1=有効, 0=無効）
        // #define kCustomSignalQuality 15000                                    // 信号品質
        // #define kCustomDetectionOnGpio 0                                      // GPIO出力による検出（1=有効, 0=無効）
        
        // I2C通信設定
        constexpr gpio_num_t kI2cMasterSclIo = GPIO_NUM_22; 
        constexpr gpio_num_t kI2cMasterSdaIo = GPIO_NUM_21; 
        constexpr i2c_port_t kI2cMasterNum = I2C_NUM_0;
        constexpr uint32_t kI2cMasterFreqHz = 400000;
        constexpr int kI2cMasterTimeoutMs = 1000;
        constexpr uint8_t kXm125Address = 0x52;
        
        // XM125固有のGPIO設定
        constexpr gpio_num_t kWakeupPin = GPIO_NUM_25;
        constexpr gpio_num_t kInterruptPin = GPIO_NUM_26;
        constexpr int kWakeupMaxRetries = 3;                     ///< ウェイクアップ最大リトライ回数
        
        const char *kTag = "Xm125SensorService";                 ///< ログ出力用タグ

    }

    /**
     * @brief コンストラクタ
     * @param handler センサータスクハンドラへのポインタ
     * @details XM125センサーサービスのインスタンスを生成します。
     */
    Xm125SensorService::Xm125SensorService(Task::ISensorTaskHandler *handler)
        : handler_(handler),
          i2cInitialized_(false),
          sensorInitialized_(false),
          busHandle_(nullptr),
          devHandle_(nullptr) {}

    /**
     * @brief デストラクタ
     * @details 使用したリソースを依存関係の逆順で解放します。
     */
    Xm125SensorService::~Xm125SensorService()
    {
        // センサーの終了処理
        if (sensorInitialized_) {
            xm125_deinit(&xm125Handle_);
        }
        
        // I2Cデバイスの削除
        if (devHandle_ != nullptr) {
            i2c_master_bus_rm_device(devHandle_);
        }
        
        // I2Cバスの削除
        if (busHandle_ != nullptr) {
            i2c_del_master_bus(busHandle_);
        }
        
        ESP_LOGW(kTag, "XM125 service resources cleaned up");
    }

    /**
     * @brief サービスの初期化
     * @retval true 初期化成功
     * @retval false 初期化失敗
     * @details I2C通信とXM125センサーの初期化を順次実行します。
     */
    bool Xm125SensorService::Initialize()
    {
        if (!InitializeI2c()) 
            return false;
        if (!InitializeXm125()) 
            return false;
        return true;
    }

    /**
     * @brief センサーデータの周期的な取得と送信
     * @details プレゼンス検出結果を読み取り、ログ出力します。
     */
    void Xm125SensorService::Run()
    {
        xm125_presence_result_t result;
        esp_err_t ret = xm125_read_presence_result(&xm125Handle_, &result);
        
        if (ret == ESP_OK) {
            // プレゼンス検出時のみINFOレベルでログ出力
            if (result.presence_detected || result.presence_detected_sticky) {
                ESP_LOGI(kTag, "Presence detected - Distance: %ld mm, Intra: %ld, Inter: %ld",
                         result.distance_mm, result.intra_score, result.inter_score);
            } else {
                // 検出なしの場合はDEBUGレベル（通常は非表示）
                ESP_LOGD(kTag, "No presence detected");
            }
            
            // センサーデータをJSON化して送信
            SendXm125SensorData(&result);
        } else {
            ESP_LOGE(kTag, "Failed to read presence result: %s", esp_err_to_name(ret));
        }
    }

    /**
     * @brief センサーデータをJSON化・パケット化して送信
     * @param result プレゼンス検出結果構造体へのポインタ
     * @details 検出結果をJSONに変換してバイナリパケット化し、送信します
     */
    void Xm125SensorService::SendXm125SensorData(const xm125_presence_result_t *result)
    {
        // センサーデータをMapに変換
        Domain::CommandInterface::SensorDataMap dataMap;
        dataMap["detected"] = result->presence_detected;                  // 現在の検出状態
        dataMap["detected_sticky"] = result->presence_detected_sticky;    // 検出履歴（タイムアウトまで保持）
        dataMap["detector_error"] = result->detector_error;               // エラーフラグ
        dataMap["distance_mm"] = result->distance_mm;                     // 検出距離（検出なし時は-1）
        dataMap["intra_score"] = result->intra_score;                     // フレーム内スコア
        dataMap["inter_score"] = result->inter_score;                     // フレーム間スコア

        // JSON文字列生成
        auto msg = Domain::CommandInterface::SensorDataMessage::Create("XM125", dataMap);
        std::string json = msg->ToJsonString();

        // バイナリパケット化
        std::vector<uint8_t> jsonBytes(json.begin(), json.end());
        auto packet = Domain::CommandInterface::CommunicationPacket::Create(
            static_cast<uint16_t>(jsonBytes.size()),
            static_cast<uint8_t>(Domain::CommandInterface::PacketType::SensorData),
            jsonBytes);
        std::vector<uint8_t> bin = packet->ToBinary();

        // タスク層のメッセージバッファに送信
        if (handler_) {
            handler_->SendSensorData(bin.data(), bin.size());
        }
    }

    /**
     * @brief I2Cバスの初期化
     * @retval true 初期化成功
     * @retval false 初期化失敗
     * @details I2Cマスターバスとデバイスを初期化します。
     * @note 多重初期化を防ぐため、既に初期化済みの場合は何もせず true を返します。
     */
    bool Xm125SensorService::InitializeI2c()
    {
        // 多重初期化チェック
        if (i2cInitialized_) return true;

        // I2Cマスターバスの設定
        i2c_master_bus_config_t busConfig = {};
        busConfig.i2c_port = kI2cMasterNum;                      // I2Cポート番号
        busConfig.sda_io_num = kI2cMasterSdaIo;                  // SDAピン
        busConfig.scl_io_num = kI2cMasterSclIo;                  // SCLピン
        busConfig.clk_source = I2C_CLK_SRC_DEFAULT;              // デフォルトクロックソース
        busConfig.glitch_ignore_cnt = 7;                         // ノイズ除去（7回連続同値で有効）
        busConfig.flags.enable_internal_pullup = true;           // 内部プルアップ抵抗を有効化

        // I2Cマスターバスの作成
        if (i2c_new_master_bus(&busConfig, &busHandle_) != ESP_OK) {
            ESP_LOGE(kTag, "Failed to initialize I2C bus");
            return false;
        }

        // XM125デバイスの設定
        i2c_device_config_t devConfig = {};
        devConfig.dev_addr_length = I2C_ADDR_BIT_LEN_7;          // 7ビットアドレス
        devConfig.device_address = kXm125Address;                // XM125のI2Cアドレス
        devConfig.scl_speed_hz = kI2cMasterFreqHz;               // SCLクロック周波数

        // I2Cバスにデバイスを追加
        if (i2c_master_bus_add_device(busHandle_, &devConfig, &devHandle_) != ESP_OK) {
            ESP_LOGE(kTag, "Failed to add XM125 device");
            return false;
        }

        i2cInitialized_ = true;
        ESP_LOGI(kTag, "I2C initialized successfully");
        return true;
    }

    
    /**
     * @brief XM125センサーの初期化
     * @retval true 初期化成功
     * @retval false 初期化失敗
     * @details 以下の処理を順次実行します：
     *          1. ドライバハンドルの設定（I2C、タイムアウト、GPIOピン）
     *          2. センサードライバの初期化
     *          3. ウェイクアップ待機（最大3回リトライ、500ms間隔）
     *          4. 測定パラメータの設定（デフォルト値 + カスタム値）
     *          5. プレゼンス検出の開始
     * @note 多重初期化を防ぐため、既に初期化済みの場合は何もせず true を返します。
     */
    bool Xm125SensorService::InitializeXm125()
    {
        // 多重初期化チェック
        if (sensorInitialized_) return true;

        // ドライバハンドルの設定
        xm125_config_t xm125Config = {};
        xm125Config.dev_handle = devHandle_;                     // I2C通信用ハンドル
        xm125Config.timeout_ms = kI2cMasterTimeoutMs;            // I2Cタイムアウト
        xm125Config.wakeup_pin = kWakeupPin;                     // ウェイクアップ制御ピン
        xm125Config.interrupt_pin = kInterruptPin;               // 割り込み検知ピン

        // センサードライバの初期化（GPIOピン設定を含む）
        esp_err_t ret = xm125_init(&xm125Handle_, &xm125Config);
        if (ret != ESP_OK) {
            ESP_LOGE(kTag, "Failed to initialize XM125: %s", esp_err_to_name(ret));
            return false;
        }

        // ウェイクアップ待機（リトライあり）
        // XM125はスリープ状態から起動するため、起動完了を待つ必要がある
        bool wakeup_success = false;
        for (int retry = 0; retry < kWakeupMaxRetries; retry++) {
            ret = xm125_wait_for_wakeup(&xm125Handle_);
            if (ret == ESP_OK) {
                wakeup_success = true;
                break;
            }
            ESP_LOGW(kTag, "Wakeup retry %d/%d", retry + 1, kWakeupMaxRetries);
            handler_->DelayMs(500);  // 500ms待ってリトライ
        }
        
        if (!wakeup_success) {
            ESP_LOGE(kTag, "Failed to wakeup XM125 after %d retries", kWakeupMaxRetries);
            return false;
        }

        // 測定設定の初期化
        xm125_measurement_config_t measureConfig = XM125_DEFAULT_MEASUREMENT_CONFIG();

        // カスタマイズしたいパラメータだけここで上書き（レジスタアドレス順）
        // #define で定義されているものだけ上書きされる
        #ifdef kCustomSweepsPerFrame
        measureConfig.sweeps_per_frame = kCustomSweepsPerFrame;
        #endif
        #ifdef kCustomInterFramePresenceTimeout
        measureConfig.inter_frame_presence_timeout = kCustomInterFramePresenceTimeout;
        #endif
        #ifdef kCustomInterPhaseBoostEnabled
        measureConfig.inter_phase_boost_enabled = kCustomInterPhaseBoostEnabled;
        #endif
        #ifdef kCustomIntraDetectionEnabled
        measureConfig.intra_detection_enabled = kCustomIntraDetectionEnabled;
        #endif
        #ifdef kCustomInterDetectionEnabled
        measureConfig.inter_detection_enabled = kCustomInterDetectionEnabled;
        #endif
        #ifdef kCustomFrameRate
        measureConfig.frame_rate = kCustomFrameRate;
        #endif
        #ifdef kCustomIntraDetectionThreshold
        measureConfig.intra_detection_threshold = kCustomIntraDetectionThreshold;
        #endif
        #ifdef kCustomInterDetectionThreshold
        measureConfig.inter_detection_threshold = kCustomInterDetectionThreshold;
        #endif
        #ifdef kCustomInterFrameDeviationTimeConst
        measureConfig.inter_frame_deviation_time_const = kCustomInterFrameDeviationTimeConst;
        #endif
        #ifdef kCustomInterFrameFastCutoff
        measureConfig.inter_frame_fast_cutoff = kCustomInterFrameFastCutoff;
        #endif
        #ifdef kCustomInterFrameSlowCutoff
        measureConfig.inter_frame_slow_cutoff = kCustomInterFrameSlowCutoff;
        #endif
        #ifdef kCustomIntraFrameTimeConst
        measureConfig.intra_frame_time_const = kCustomIntraFrameTimeConst;
        #endif
        #ifdef kCustomIntraOutputTimeConst
        measureConfig.intra_output_time_const = kCustomIntraOutputTimeConst;
        #endif
        #ifdef kCustomInterOutputTimeConst
        measureConfig.inter_output_time_const = kCustomInterOutputTimeConst;
        #endif
        #ifdef kCustomAutoProfileEnabled
        measureConfig.auto_profile_enabled = kCustomAutoProfileEnabled;
        #endif
        #ifdef kCustomAutoStepLengthEnabled
        measureConfig.auto_step_length_enabled = kCustomAutoStepLengthEnabled;
        #endif
        #ifdef kCustomManualProfile
        measureConfig.manual_profile = kCustomManualProfile;
        #endif
        #ifdef kCustomManualStepLength
        measureConfig.manual_step_length = kCustomManualStepLength;
        #endif
        #ifdef kCustomStartMm
        measureConfig.start_mm = kCustomStartMm;
        #endif
        #ifdef kCustomEndMm
        measureConfig.end_mm = kCustomEndMm;
        #endif
        #ifdef kCustomResetFiltersOnPrepare
        measureConfig.reset_filters_on_prepare = kCustomResetFiltersOnPrepare;
        #endif
        #ifdef kCustomHwaas
        measureConfig.hwaas = kCustomHwaas;
        #endif
        #ifdef kCustomAutomaticSubsweeps
        measureConfig.automatic_subsweeps = kCustomAutomaticSubsweeps;
        #endif
        #ifdef kCustomSignalQuality
        measureConfig.signal_quality = kCustomSignalQuality;
        #endif
        #ifdef kCustomDetectionOnGpio
        measureConfig.detection_on_gpio = kCustomDetectionOnGpio;
        #endif

        // 測定設定をセンサーに適用
        ret = xm125_apply_measurement_config(&xm125Handle_, &measureConfig);
        if (ret != ESP_OK) {
            ESP_LOGE(kTag, "Failed to apply measurement config: %s", esp_err_to_name(ret));
            return false;
        }

        // プレゼンス検出開始
        ret = xm125_start_detector(&xm125Handle_);
        if (ret != ESP_OK) {
            ESP_LOGE(kTag, "Failed to start detector: %s", esp_err_to_name(ret));
            return false;
        }

        sensorInitialized_ = true;
        ESP_LOGI(kTag, "XM125 sensor initialized successfully");
        return true;
    }
}
