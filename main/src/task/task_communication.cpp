/**
 * @file task_communication.cpp
 * @brief PC間通信を行うタスクの実装ファイル
 *
 * @copyright Copyright (C) 2026, Heartland Data inc. All Rights Reserved.
 *
 * SPDX-FileCopyrightText: 2026 Heartland Data inc.
 * SPDX-License-Identifier: Apache-2.0
 */

#include "task_communication.hpp"
#include "esp_log.h"
#include "service/uart_communication_service.hpp"
#include "service/bt_communication_service.hpp"
#include <cstring> // memset用
// DT_INCLUDE

namespace Task
{
    /**
     * @brief TaskCommunication のコンストラクタ
     * @param rx_buffer 受信データ用メッセージバッファのハンドル
     * @param tx_buffer 送信データ用メッセージバッファのハンドル
     */
    TaskCommunication::TaskCommunication(MessageBufferHandle_t rxBuffer, MessageBufferHandle_t txBuffer)
        : AbstractTask(kName, kStackDepth, kPriority, kIntervalMsec), rxBuffer_(rxBuffer), txBuffer_(txBuffer), txMessageCacheLen_(0), txMessageCache_(0) {}

    /**
     * @brief タスクのメイン処理。通信サービスの初期化と周期実行を行う。
     */
    void TaskCommunication::Run()
    {
        ESP_LOGI(kName, "TaskCommunication is running");
        communicationService_ = new service::UARTCommunicationService(this);
        btCommunicationService_ = new service::BtCommunicationService(this);

        bool uartInit = communicationService_ && communicationService_->Initialize();
        bool btInit = btCommunicationService_ && btCommunicationService_->Initialize();

        if (!uartInit)
        {
            ESP_LOGE(kName, "Failed to initialize UART communication service, task terminating");
            return;
        }
        if (!btInit)
        {
            ESP_LOGE(kName, "Failed to initialize Bluetooth communication service, continue with UART only");
        }
        ESP_LOGI(kName, "Communication services initialized successfully");

        for (;;)
        {
            if (txBuffer_)
            {
                txMessageCacheLen_ = 0;
                memset(txMessageCache_, 0, sizeof(txMessageCache_));
                // 送信するデータを一時的に保持する
                // 保持したデータを各サービスから参照して送信する
                txMessageCacheLen_ = xMessageBufferReceive(txBuffer_, txMessageCache_, kTxMessageBufferSize, 0);
                txMessageCache_[txMessageCacheLen_] = '\0';
            }

            // 送受信処理はサービス側で行う。Task側でメッセージバッファ管理を行う。
            communicationService_->Run();
            if (btInit)
            {
                btCommunicationService_->Run();
            }

            vTaskDelay(static_cast<TickType_t>(kIntervalMsec / portTICK_PERIOD_MS));
        }
    }

    /**
     * @brief 送信メッセージキャッシュの先頭アドレスを取得する
     * @return const uint8_t* 送信メッセージキャッシュの先頭アドレス
     */
    const uint8_t *TaskCommunication::GetTxMessageCache() const
    {
        return txMessageCache_;
    }

    /**
     * @brief 送信メッセージキャッシュのデータ長を取得する
     * @return size_t 送信メッセージキャッシュのデータ長
     */
    size_t TaskCommunication::GetTxMessageCacheLen() const
    {
        return txMessageCacheLen_;
    }

    /**
     * @brief 受信データをメッセージバッファに送信する。
     * @param data 受信データ
     * @param len データ長
     */
    void TaskCommunication::OnReceive(const uint8_t *data, size_t len)
    {
        if (rxBuffer_ && data && len > 0)
        {
            xMessageBufferSend(rxBuffer_, data, len, 0);
        }
    }

} // namespace Task
