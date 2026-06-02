/**
 * @file mlx90640_ir_sensor_service.cpp
 * @brief MLX90640用IRセンササービス実装
 * @copyright Copyright (C) 2026, Heartland Data inc. All Rights Reserved.
 * SPDX-FileCopyrightText: 2026 Heartland Data inc.
 * SPDX-License-Identifier: Apache-2.0
 */

#include "mlx90640_ir_sensor_service.hpp"
#include "esp_log.h"
#include <cstdio>
#include <vector>
#include <cstring>
#include <cmath>
#include "command_interface/communication_packet.hpp"
#include "command_interface/sensor_data_message.hpp"

namespace service
{

    namespace
    {
        constexpr gpio_num_t kI2cMasterSclIo = GPIO_NUM_22;
        constexpr gpio_num_t kI2cMasterSdaIo = GPIO_NUM_21;
        constexpr i2c_port_t kI2cMasterNum = I2C_NUM_0;
        constexpr uint32_t kI2cMasterFreqHz = 400000;
        constexpr uint8_t kMlx90640Address = 0x33;
        constexpr uint8_t kDefaultRefreshRate = 0x03; // ~4Hz
        constexpr uint8_t kDefaultResolution = 0x01;  // 18-bit
        // MLX90640のピクセル配列サイズ（ヘッダーの定数を使用）
        constexpr int kPixelRows = MLX90640Spec::kPixelRows;
        constexpr int kPixelCols = MLX90640Spec::kPixelCols;
        constexpr int kPixelArraySize = MLX90640Spec::kPixelCount; // 768 pixels in total
        constexpr int kRowBufSize = 256;
        const char *kTag = "Mlx90640IrSensorService";
    } // namespace

    /**
     * @brief コンストラクタ
     * @details MLX90640 IRセンササービスのインスタンスを生成します。
     */
    Mlx90640IrSensorService::Mlx90640IrSensorService(Task::ISensorTaskHandler *handler)
        : handler_(handler),
          deviceAddress_(kMlx90640Address),
          i2cPort_(kI2cMasterNum),
          sdaPin_(static_cast<int>(kI2cMasterSdaIo)),
          sclPin_(static_cast<int>(kI2cMasterSclIo)),
          i2cFreqHz_(kI2cMasterFreqHz),
          refreshRate_(kDefaultRefreshRate),
          resolution_(kDefaultResolution),
          sensorInitialized_(false)
    {
        // Initialize arrays
        std::memset(&params_, 0, sizeof(params_));
        std::memset(frameData_, 0, sizeof(frameData_));
        std::memset(eeData_, 0, sizeof(eeData_));
    }

    /**
     * @brief デストラクタ
     * @details 使用したリソースを解放します。
     */
    Mlx90640IrSensorService::~Mlx90640IrSensorService()
    {
        if (sensorInitialized_)
        {
            MLX90640_I2CDeinit(i2cPort_);
        }
        ESP_LOGW(kTag, "MLX90640 service resources cleaned up");
    }

    /**
     * @brief サービスの初期化
     * @retval true 初期化成功
     * @retval false 初期化失敗
     * @details MLX90640センサの初期化を行います。
     */
    bool Mlx90640IrSensorService::Initialize()
    {
        if (!InitializeMlx90640())
            return false;
        return true;
    }

    /**
     * @brief センサデータの取得・出力処理
     * @details 温度データおよび周囲温度を取得し、ログ出力します。
     */
    void Mlx90640IrSensorService::Run()
    {
        float pixels[kPixelArraySize];
        float ambientTemp;

        for (size_t expectedSubPage = 0; expectedSubPage < 2; expectedSubPage++)
        {
            uint8_t retry = 0;
            const uint8_t kMaxRetry = 10;
            bool gotSubPage = false;
            do
            {
                memset(frameData_, 0, sizeof(frameData_));
                int ret = MLX90640_GetFrameData(deviceAddress_, frameData_);
                if (ret < 0)
                {
                    ESP_LOGE(kTag, "Failed to get frame data: %d", ret);
                    return;
                }

                ESP_LOGI(kTag, "SubPage: %d", MLX90640_GetSubPageNumber(frameData_));
                if (MLX90640_GetSubPageNumber(frameData_) == expectedSubPage)
                {
                    gotSubPage = true;
                    break;
                }

                if (handler_)
                {
                    handler_->DelayMs(1);
                }
            } while (++retry < kMaxRetry);

            if (!gotSubPage)
            {
                ESP_LOGE(kTag, "Failed to get expected SubPage %d after %d retries", (int)expectedSubPage, kMaxRetry);
                return;
            }

            MLX90640_GetVdd(frameData_, &params_);
            ambientTemp = MLX90640_GetTa(frameData_, &params_);
            ESP_LOGD(kTag, "Ambient temperature: %.2f°C", ambientTemp);
            float tr = ambientTemp - 8.0f; // Default TA_SHIFT
            float emissivity = 0.95f;      // Default emissivity
            MLX90640_CalculateTo(frameData_, &params_, emissivity, tr, pixels);
        }

        // センサーデータ送信
        this->SendMlx90640SensorData(pixels, kPixelArraySize, ambientTemp);
    }

    /**
     * @brief MLX90640センサデータをJSON化・パケット化して送信
     * @param pixels ピクセル温度配列
     * @param pixelCount 配列サイズ
     * @param ambientTemp 周囲温度
     */
    void Mlx90640IrSensorService::SendMlx90640SensorData(const float *pixels, size_t pixelCount, float ambientTemp)
    {
        // 24x32配列へ変換
        // 定数はnamespace内で定義済み
        std::vector<std::vector<double>> temperature(kPixelRows, std::vector<double>(kPixelCols));
        for (int row = 0; row < kPixelRows; ++row)
        {
            for (int col = 0; col < kPixelCols; ++col)
            {
                double value = static_cast<double>(pixels[row * kPixelCols + col]);
                temperature[row][col] = std::round(value * 100.0) / 100.0;
            }
        }

        // センサデータマップ作成
        Domain::CommandInterface::SensorDataMap dataMap;
        dataMap["temperature"] = std::move(temperature);
        dataMap["ambient"] = static_cast<double>(std::round(ambientTemp * 100.0) / 100.0);

        // SensorDataMessage生成
        auto msg = Domain::CommandInterface::SensorDataMessage::Create("MLX90640", dataMap);
        std::string json = msg->ToJsonString();

        // パケット化
        std::vector<uint8_t> jsonBytes(json.begin(), json.end());
        auto packet = Domain::CommandInterface::CommunicationPacket::Create(
            static_cast<uint16_t>(jsonBytes.size()),
            static_cast<uint8_t>(Domain::CommandInterface::PacketType::SensorData),
            jsonBytes);
        std::vector<uint8_t> bin = packet->ToBinary();

        // 送信
        if (handler_)
        {
            handler_->SendSensorData(bin.data(), bin.size());
        }
    }

    /**
     * @brief MLX90640センサの初期化
     * @retval true 初期化成功
     * @retval false 初期化失敗
     * @details MLX90640センサの初期化と設定を行います。
     */
    bool Mlx90640IrSensorService::InitializeMlx90640()
    {
        if (sensorInitialized_)
            return true;

        // I2C初期化
        int ret = MLX90640_I2CInit(i2cPort_, sdaPin_, sclPin_, i2cFreqHz_, deviceAddress_);
        if (ret != ESP_OK)
        {
            ESP_LOGE(kTag, "Failed to initialize I2C");
            return false;
        }

        // センサ準備待ち
        if (handler_)
        {
            handler_->DelayMs(100);
        }
        else
        {
            ESP_LOGW(kTag, "Handler is null, skipping sensor preparation delay");
        }

        // EEPROM データ読み込み
        ret = MLX90640_DumpEE(deviceAddress_, eeData_);
        if (ret != 0)
        {
            ESP_LOGE(kTag, "Failed to read EEPROM data: %d", ret);
            MLX90640_I2CDeinit(i2cPort_);
            return false;
        }

        // パラメータ抽出
        ret = MLX90640_ExtractParameters(eeData_, &params_);
        if (ret != 0)
        {
            ESP_LOGE(kTag, "Failed to extract parameters: %d", ret);
            MLX90640_I2CDeinit(i2cPort_);
            return false;
        }

        // リフレッシュレート設定
        ret = MLX90640_SetRefreshRate(deviceAddress_, refreshRate_);
        if (ret != 0)
        {
            ESP_LOGE(kTag, "Failed to set refresh rate: %d", ret);
            MLX90640_I2CDeinit(i2cPort_);
            return false;
        }

        // 解像度設定
        ret = MLX90640_SetResolution(deviceAddress_, resolution_);
        if (ret != 0)
        {
            ESP_LOGE(kTag, "Failed to set resolution: %d", ret);
            MLX90640_I2CDeinit(i2cPort_);
            return false;
        }

        // チェスモード設定
        ret = MLX90640_SetChessMode(deviceAddress_);
        if (ret != 0)
        {
            ESP_LOGE(kTag, "Failed to set chess mode: %d", ret);
            MLX90640_I2CDeinit(i2cPort_);
            return false;
        }

        sensorInitialized_ = true;
        ESP_LOGI(kTag, "MLX90640 sensor initialized successfully");
        return true;
    }

} // namespace service
