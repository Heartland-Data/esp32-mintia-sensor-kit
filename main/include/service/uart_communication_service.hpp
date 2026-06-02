/**
 * @file uart_communication_service.hpp
 * @brief UART通信サービスのインターフェース
 *
 * @copyright Copyright (C) 2026, Heartland Data inc. All Rights Reserved.
 *
 * SPDX-FileCopyrightText: 2026 Heartland Data inc.
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once
#include "service/icommunication_service.hpp"
#include "driver/uart.h"
#include "task/icommunication_task_handler.hpp"
#include "task/message_buffer_defs.hpp"

namespace service
{

    /**
     * @brief UART通信サービスのインターフェース
     */
    class UARTCommunicationService : public ICommunicationService
    {
    public:
        UARTCommunicationService(Task::ICommunicationTaskHandler *handler);
        ~UARTCommunicationService() override;

        bool Initialize() override;
        void Run() override;

    private:
        Task::ICommunicationTaskHandler *handler_;

        // 送受信バッファ（メンバ変数として定義）
        uint8_t rxBuffer_[kRxMessageBufferSize];

        void ProcessSendData();
        void ProcessReceivedData();
    };

} // namespace service
