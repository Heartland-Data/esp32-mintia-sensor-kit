/**
 * @file amg88xx_ir_sensor_service.cpp
 * @brief AMG88xx用IRセンササービス実装
 * @copyright Copyright (C) 2026, Heartland Data inc. All Rights Reserved.
 * SPDX-FileCopyrightText: 2026 Heartland Data inc.
 * SPDX-License-Identifier: Apache-2.0
 */

#include "amg88xx_ir_sensor_service.hpp"
#include "esp_log.h"
#include <cstdio>
#include <vector>
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
    constexpr int kI2cMasterTimeoutMs = 1000;
    constexpr uint8_t kAmg88xxAddress = 0x68;
    constexpr int kPixelRows = 8;
    constexpr int kPixelCols = 8;
    constexpr int kPixelArraySize = kPixelRows * kPixelCols; // 64 pixels in total
    constexpr int kRowBufSize = 128;
    const char *kTag = "Amg88xxIrSensorService";
  } // namespace

  /**
   * @brief コンストラクタ
   * @details AMG88xx IRセンササービスのインスタンスを生成します。
   */
  Amg88xxIrSensorService::Amg88xxIrSensorService(Task::ISensorTaskHandler *handler)
      : handler_(handler),
        i2cInitialized_(false),
        sensorInitialized_(false),
        busHandle_(nullptr),
        devHandle_(nullptr) {}

  /**
   * @brief デストラクタ
   * @details 使用したリソースを解放します。
   */
  Amg88xxIrSensorService::~Amg88xxIrSensorService()
  {
    if (sensorInitialized_)
    {
      amg88xx_deinit(&amgHandle_);
    }
    if (devHandle_ != nullptr)
    {
      i2c_master_bus_rm_device(devHandle_);
    }
    if (busHandle_ != nullptr)
    {
      i2c_del_master_bus(busHandle_);
    }
    ESP_LOGW(kTag, "AMG88xx service resources cleaned up");
  }

  /**
   * @brief サービスの初期化
   * @retval true 初期化成功
   * @retval false 初期化失敗
   * @details I2CバスおよびAMG88xxセンサの初期化を行います。
   */
  bool Amg88xxIrSensorService::Initialize()
  {
    if (!InitializeI2c())
      return false;
    if (!InitializeAmg88xx())
      return false;
    return true;
  }

  /**
   * @brief センサデータの取得・出力処理
   * @details サーミスタ温度およびピクセル温度を取得し、ログ出力します。
   */
  void Amg88xxIrSensorService::Run()
  {
    float pixels[kPixelArraySize];
    float thermistorTemp;
    esp_err_t ret;

    // サーミスタ温度取得
    ret = amg88xx_read_thermistor(&amgHandle_, &thermistorTemp);
    if (ret == ESP_OK)
    {
      ESP_LOGD(kTag, "Thermistor temperature: %.2f°C", thermistorTemp);
    }
    else
    {
      ESP_LOGE(kTag, "Failed to read thermistor: %s", esp_err_to_name(ret));
    }

    // ピクセル温度取得
    ret = amg88xx_read_pixels(&amgHandle_, pixels, kPixelArraySize);
    if (ret == ESP_OK)
    {
      ESP_LOGD(kTag, "Thermal image (°C):");
      for (int row = 0; row < kPixelRows; ++row)
      {
        char rowBuf[kRowBufSize] = {0};
        int offset = 0;
        offset += snprintf(rowBuf + offset, sizeof(rowBuf) - offset, "Row %d: ", row);
        for (int col = 0; col < kPixelCols; ++col)
        {
          offset += snprintf(rowBuf + offset, sizeof(rowBuf) - offset, "%6.2f ", pixels[row * kPixelCols + col]);
        }
        ESP_LOGD(kTag, "%s", rowBuf);
      }
      // センサーデータ送信
      this->SendAmg88xxSensorData(pixels, kPixelArraySize, thermistorTemp);
    }
    else
    {
      ESP_LOGE(kTag, "Failed to read pixels: %s", esp_err_to_name(ret));
    }
  }

  /**
   * @brief AMG88xxセンサデータをJSON化・パケット化して送信
   * @param pixels ピクセル温度配列
   * @param pixelCount 配列サイズ
   * @param thermistorTemp サーミスタ温度
   */
  void Amg88xxIrSensorService::SendAmg88xxSensorData(const float *pixels, size_t pixelCount, float thermistorTemp)
  {
    // 8x8配列へ変換
    // 定数はnamespace内で定義済み
    std::vector<std::vector<double>> temperature(kPixelRows, std::vector<double>(kPixelCols));
    for (int row = 0; row < kPixelRows; ++row)
    {
      for (int col = 0; col < kPixelCols; ++col)
      {
        temperature[row][col] = static_cast<double>(pixels[row * kPixelCols + col]);
      }
    }

    // センサデータマップ作成
    Domain::CommandInterface::SensorDataMap dataMap;
    dataMap["temperature"] = temperature;
    dataMap["ambient"] = static_cast<double>(thermistorTemp);

    // SensorDataMessage生成
    auto msg = Domain::CommandInterface::SensorDataMessage::Create("AMG8833", dataMap);
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
   * @brief I2Cバスの初期化
   * @retval true 初期化成功
   * @retval false 初期化失敗
   * @details I2Cバスおよびデバイスの登録を行います。
   */
  bool Amg88xxIrSensorService::InitializeI2c()
  {
    if (i2cInitialized_)
      return true;

    i2c_master_bus_config_t busConfig = {};
    busConfig.i2c_port = kI2cMasterNum;
    busConfig.sda_io_num = kI2cMasterSdaIo;
    busConfig.scl_io_num = kI2cMasterSclIo;
    busConfig.clk_source = I2C_CLK_SRC_DEFAULT;
    busConfig.glitch_ignore_cnt = 7;
    busConfig.flags.enable_internal_pullup = true;

    if (i2c_new_master_bus(&busConfig, &busHandle_) != ESP_OK)
    {
      ESP_LOGE(kTag, "Failed to initialize I2C bus");
      return false;
    }

    i2c_device_config_t devConfig = {};
    devConfig.dev_addr_length = I2C_ADDR_BIT_LEN_7;
    devConfig.device_address = kAmg88xxAddress;
    devConfig.scl_speed_hz = kI2cMasterFreqHz;

    if (i2c_master_bus_add_device(busHandle_, &devConfig, &devHandle_) != ESP_OK)
    {
      ESP_LOGE(kTag, "Failed to add AMG88xx device");
      return false;
    }

    i2cInitialized_ = true;
    ESP_LOGI(kTag, "I2C initialized successfully");
    return true;
  }

  /**
   * @brief AMG88xxセンサの初期化
   * @retval true 初期化成功
   * @retval false 初期化失敗
   * @details AMG88xxセンサの初期化と設定を行います。
   */
  bool Amg88xxIrSensorService::InitializeAmg88xx()
  {
    if (sensorInitialized_)
      return true;

    amg88xx_config_t amgConfig = {};
    amgConfig.dev_handle = devHandle_;
    amgConfig.timeout_ms = kI2cMasterTimeoutMs;

    esp_err_t ret = amg88xx_init(&amgHandle_, &amgConfig);
    if (ret != ESP_OK)
    {
      ESP_LOGE(kTag, "Failed to initialize AMG88xx: %s", esp_err_to_name(ret));
      return false;
    }

    ret = amg88xx_set_frame_rate(&amgHandle_, AMG88XX_FPS_10);
    if (ret != ESP_OK)
    {
      ESP_LOGE(kTag, "Failed to set frame rate: %s", esp_err_to_name(ret));
      return false;
    }

    ret = amg88xx_set_moving_average_mode(&amgHandle_, false);
    if (ret != ESP_OK)
    {
      ESP_LOGE(kTag, "Failed to set moving average mode: %s", esp_err_to_name(ret));
      return false;
    }

    sensorInitialized_ = true;
    ESP_LOGI(kTag, "AMG88xx sensor initialized successfully");
    return true;
  }

} // namespace service
