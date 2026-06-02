/**
 * @file uart_communication_service.cpp
 * @brief UART通信サービスの実装
 *
 * @copyright Copyright (C) 2026, Heartland Data inc. All Rights Reserved.
 *
 * SPDX-FileCopyrightText: 2026 Heartland Data inc.
 * SPDX-License-Identifier: Apache-2.0
 */

#include "service/uart_communication_service.hpp"
#include "esp_log.h"
#include <cstring> // memsetのため
// DT_INCLUDE

namespace service
{

    // ファイルスコープ定数
    namespace
    {
        constexpr uart_port_t kUartNum = UART_NUM_0;
        constexpr int kTxPin = 1; // TXD0
        constexpr int kRxPin = 3; // RXD0
        constexpr int kBaudRate = 115200;
        const char *kTag = "UartCommunicationService";
    }

    /**
     * @brief UARTCommunicationService のコンストラクタ
     */
    UARTCommunicationService::UARTCommunicationService(Task::ICommunicationTaskHandler *handler)
        : handler_(handler), rxBuffer_(0)
    {
    }

    /**
     * @brief UARTCommunicationService のデストラクタ
     */
    UARTCommunicationService::~UARTCommunicationService()
    {
    }

    /**
     * @brief UART通信の初期化を行う
     *
     * @return true 初期化成功
     * @return false 初期化失敗
     */
    bool UARTCommunicationService::Initialize()
    {
        // UART初期化処理
        uart_config_t uartConfig = {
            .baud_rate = kBaudRate,
            .data_bits = UART_DATA_8_BITS,
            .parity = UART_PARITY_DISABLE,
            .stop_bits = UART_STOP_BITS_1,
            .flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
            .rx_flow_ctrl_thresh = 122,
            .source_clk = UART_SCLK_DEFAULT,
            .flags = {
                .allow_pd = 0,
                .backup_before_sleep = 0,
            },
        };

        if (uart_param_config(kUartNum, &uartConfig) != ESP_OK)
        {
            return false;
        }
        if (uart_set_pin(kUartNum, kTxPin, kRxPin, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE) != ESP_OK)
        {
            return false;
        }
        if (uart_driver_install(kUartNum, kRxMessageBufferSize, kTxMessageBufferSize, 0, NULL, 0) != ESP_OK)
        {
            return false;
        }
        return true;
    }

    /**
     * @brief UART通信のメイン処理
     */
    void UARTCommunicationService::Run()
    {
        ProcessSendData();
        ProcessReceivedData();
    }

    /**
     * @brief 送信データの処理
     *
     */
    void UARTCommunicationService::ProcessSendData()
    {
        if (!handler_)
        {
            return;
        }

        const uint8_t *txBuf = handler_->GetTxMessageCache();
        size_t txLen = handler_->GetTxMessageCacheLen();
        if (txLen == 0)
        {
            ESP_LOGD(kTag, "No data to send (ProcessSendData)");
            return;
        }

        // 送信データが文字列である保証がない場合は16進ダンプ等に変更も検討
        ESP_LOGI(kTag, "Sending %zu bytes (ProcessSendData)", txLen);
        uart_write_bytes(kUartNum, (const char *)txBuf, txLen);
    }

    /**
     * @brief 受信データの処理
     *
     * @param onReceive 受信時コールバック
     * @param userCtx コールバックに渡すユーザコンテキスト
     */
    void UARTCommunicationService::ProcessReceivedData()
    {
        if (!handler_)
        {
            return;
        }

        // バッファをクリア
        memset(rxBuffer_, 0, sizeof(rxBuffer_));

        int readLen = uart_read_bytes(kUartNum, rxBuffer_, sizeof(rxBuffer_) - 1, 0);
        if (readLen > 0)
        {
            rxBuffer_[readLen] = '\0';
            ESP_LOGI(kTag, "Received %d bytes (ProcessReceivedData), data: %s", readLen, rxBuffer_);
            handler_->OnReceive(rxBuffer_, readLen);
        }
    }

} // namespace service
