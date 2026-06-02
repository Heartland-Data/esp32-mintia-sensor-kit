/**
 * @file task_data_parser.cpp
 * @brief データ解析タスク 実装
 * @copyright Copyright (C) 2026, Heartland Data inc. All Rights Reserved.
 * SPDX-FileCopyrightText: 2026 Heartland Data inc.
 * SPDX-License-Identifier: Apache-2.0
 */

#include "task_data_parser.hpp"
#include "esp_log.h"

namespace Task
{

    /**
     * @brief TaskDataParser のコンストラクタ
     * @param parser_service データ解析サービスのインターフェースポインタ
     * @param rx_buffer 受信データ用メッセージバッファのハンドル
     * @param tx_buffer 送信データ用メッセージバッファのハンドル
     * @param sensor_queue センサタスクへのコマンド送信用キュー
     */
    TaskDataParser::TaskDataParser(MessageBufferHandle_t rx_buffer, MessageBufferHandle_t tx_buffer, QueueHandle_t sensor_queue)
        : AbstractTask(kName, kStackDepth, kPriority, kIntervalMsec),
          rxBuffer_(rx_buffer), txBuffer_(tx_buffer), SensorQueue_(sensor_queue) {}

    /**
     * @brief タスクのメイン処理。メッセージバッファからデータを受信し、解析サービスに渡す。
     */
    void TaskDataParser::Run()
    {
        uint8_t buf[kRxMessageBufferSize];
        parserService_ = new service::DataParserService(this);
        for (;;)
        {
            // メッセージバッファからデータを受信
            // タイムアウトは portMAX_DELAY で無限待機(他のタスクに処理を譲る)
            size_t len = xMessageBufferReceive(rxBuffer_, buf, sizeof(buf), portMAX_DELAY);
            if (len > 0 && parserService_)
            {
                parserService_->Parse(buf, len);
            }
        }
    }

    /**
     * @brief センサへコマンド送信
     * @param cmd 送信するコマンド
     * @return true: 送信成功, false: 送信失敗
     */
    bool TaskDataParser::SendSensorCommand(const Domain::CommandInterface::SensorCommandMsg &cmd)
    {
        if (!SensorQueue_)
        {
            ESP_LOGE(kName, "Sensor queue is not initialized");
            return false;
        }

        BaseType_t result = xQueueSend(SensorQueue_, &cmd, 0);
        if (result != pdTRUE)
        {
            ESP_LOGW(kName, "Failed to send command to Sensor queue");
            return false;
        }

        ESP_LOGI(kName, "Command sent to Sensor: cmd=%d, param=%lu",
                 static_cast<int>(cmd.cmd), cmd.param);
        return true;
    }

    /**
     * @brief 送信データをtxMessageBufferへ送信
     * @param data 送信データ
     * @param len データ長
     * @return true: 送信成功, false: 送信失敗
     */
    bool TaskDataParser::SendResponse(const uint8_t *data, size_t len)
    {
        if (!txBuffer_ || !data || len == 0)
        {
            ESP_LOGE(kName, "Invalid parameters for SendResponse");
            return false;
        }

        size_t sent = xMessageBufferSend(txBuffer_, data, len, 0);
        if (sent != len)
        {
            ESP_LOGW(kName, "Failed to send response to tx buffer (sent: %zu, expected: %zu)", sent, len);
            return false;
        }

        ESP_LOGI(kName, "Response sent to tx buffer: %zu bytes", len);
        return true;
    }

} // namespace Task
