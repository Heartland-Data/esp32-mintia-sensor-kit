/**
 * @file task_sensor.cpp
 * @brief センサタスク（サービス抽象化）実装
 * @copyright Copyright (C) 2026, Heartland Data inc. All Rights Reserved.
 * SPDX-FileCopyrightText: 2026 Heartland Data inc.
 * SPDX-License-Identifier: Apache-2.0
 */

#include "task/task_sensor.hpp"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

// センササービス選択 (使用するセンサー以外をコメントアウト)
// #define USE_AMG88XX_SENSOR    // AMG88xxセンサを使用
// #define USE_MLX90640_SENSOR   // MLX90640センサを使用
#define USE_XM125_SENSOR      // XM125プレゼンスセンサを使用

#ifdef USE_AMG88XX_SENSOR
#include "amg88xx_ir_sensor_service.hpp"
#endif

#ifdef USE_MLX90640_SENSOR
#include "mlx90640_ir_sensor_service.hpp"
#endif

#ifdef USE_XM125_SENSOR
#include "xm125_sensor_service.hpp"
#endif

namespace Task
{

  /**
   * @brief コンストラクタ
   * @param service センササービスのインターフェースポインタ
   * @param command_queue コマンド受信用キュー
   * @param tx_buffer 送信データ用メッセージバッファ
   * @details センサタスクのインスタンスを生成します。
   */
  SensorTask::SensorTask(QueueHandle_t command_queue, MessageBufferHandle_t tx_buffer)
      : AbstractTask(kName, kStackDepth, kPriority, kIntervalMsec),
        commandQueue_(command_queue), txBuffer_(tx_buffer), isSensorRunning_(false) {}

  /**
   * @brief デストラクタ
   * @details センサタスクのリソースを解放します。
   */
  SensorTask::~SensorTask() = default;

  /**
   * @brief タスクの実行処理
   * @details センササービスの初期化と、コマンド受信・処理を行います。
   */
  void SensorTask::Run()
  {
    ESP_LOGI(kName, "Starting Sensor task");

#ifdef USE_AMG88XX_SENSOR
    service_ = new service::Amg88xxIrSensorService(this);
    ESP_LOGI(kName, "Using AMG88xx IR sensor");
#elif defined(USE_MLX90640_SENSOR)
    service_ = new service::Mlx90640IrSensorService(this);
    ESP_LOGI(kName, "Using MLX90640 IR sensor");
#elif defined(USE_XM125_SENSOR)
    service_ = new service::Xm125SensorService(this);
    ESP_LOGI(kName, "Using XM125 presence sensor");
#else
#error "No sensor selected. Please define USE_AMG88XX_SENSOR, USE_MLX90640_SENSOR, or USE_XM125_SENSOR"
#endif

    if (!service_ || !service_->Initialize())
    {
      ESP_LOGE(kName, "Failed to initialize Sensor service, task terminating");
      return;
    }
    ESP_LOGI(kName, "Sensor task initialization complete");

    for (;;)
    {
      // コマンド受信処理
      ProcessCommand();

      // サービス処理
      if (service_ && isSensorRunning_)
      {
        service_->Run();
      }

      vTaskDelay(static_cast<TickType_t>(kIntervalMsec / portTICK_PERIOD_MS));
    }
  }

  /**
   * @brief コマンド処理
   * @param cmd 受信したコマンド
   */
  void SensorTask::ProcessCommand()
  {
    Domain::CommandInterface::SensorCommandMsg cmd;
    if (!commandQueue_ || xQueueReceive(commandQueue_, &cmd, 0) != pdTRUE)
    {
      return;
    }

    ESP_LOGI(kName, "Received command: cmd=%d, param=%" PRIu32, static_cast<int>(cmd.cmd), cmd.param);

    switch (cmd.cmd)
    {
    case Domain::CommandInterface::SensorCommandType::kStart:
      ESP_LOGI(kName, "Processing START command");
      isSensorRunning_ = true;
      break;

    case Domain::CommandInterface::SensorCommandType::kStop:
      ESP_LOGI(kName, "Processing STOP command");
      isSensorRunning_ = false;
      break;

    case Domain::CommandInterface::SensorCommandType::kSetParam:
      ESP_LOGI(kName, "Processing SET_PARAM command with param=%lu", cmd.param);
      // TODO: サービス側にパラメータ設定処理を追加予定
      break;

    default:
      ESP_LOGW(kName, "Unknown command: %d", static_cast<int>(cmd.cmd));
      break;
    }
  }

  /**
   * @brief センサーデータを送信
   * @param data 送信データ
   * @param len データ長
   * @return true: 送信成功, false: 送信失敗
   */
  bool SensorTask::SendSensorData(const uint8_t *data, size_t len)
  {
    if (!txBuffer_ || !data || len == 0)
    {
      ESP_LOGE(kName, "Invalid parameters for SendSensorData");
      return false;
    }

    // バッファの空き容量をチェック
    size_t freeSpace = xMessageBufferSpaceAvailable(txBuffer_);
    ESP_LOGI(kName, "TX buffer free space: %zu bytes, trying to send: %zu bytes", freeSpace, len);

    size_t sent = xMessageBufferSend(txBuffer_, data, len, 0);
    if (sent != len)
    {
      ESP_LOGW(kName, "Failed to send sensor data to tx buffer (sent: %zu, expected: %zu)", sent, len);
      return false;
    }

    ESP_LOGI(kName, "Sensor data sent to tx buffer: %zu bytes", len);
    return true;
  }

  /**
   * @brief 遅延処理
   * @param ms 遅延時間（ミリ秒）
   */
  void SensorTask::DelayMs(uint32_t ms)
  {
    vTaskDelay(pdMS_TO_TICKS(ms));
  }

} // namespace task
