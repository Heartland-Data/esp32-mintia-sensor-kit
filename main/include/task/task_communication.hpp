/**
 * @file task_communication.hpp
 * @brief PC間通信を行うタスクのヘッダファイル
 *
 * @copyright Copyright (C) 2026, Heartland Data inc. All Rights Reserved.
 *
 * SPDX-FileCopyrightText: 2026 Heartland Data inc.
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include "abstract_task.hpp"
#include "service/icommunication_service.hpp"
#include "service/bt_communication_service.hpp"
#include "task/icommunication_task_handler.hpp"
#include "task/message_buffer_defs.hpp"

namespace Task
{
    class TaskCommunication : public AbstractTask, public ICommunicationTaskHandler
    {
    public:
        TaskCommunication(MessageBufferHandle_t rxBuffer, MessageBufferHandle_t txBuffer);
        virtual ~TaskCommunication() = default;
        void Run() override;

        // ICommunicationTaskHandlerの実装
        void OnReceive(const uint8_t *data, size_t len) override;
        const uint8_t *GetTxMessageCache() const;
        size_t GetTxMessageCacheLen() const;

    private:
        service::ICommunicationService *communicationService_;
        service::ICommunicationService *btCommunicationService_;
        MessageBufferHandle_t rxBuffer_;
        MessageBufferHandle_t txBuffer_;
        size_t txMessageCacheLen_;
        uint8_t txMessageCache_[kTxMessageBufferSize];
        static constexpr const char *const kName = "task_communication";
        static constexpr uint32_t kStackDepth = 8192;
        static constexpr int kPriority = 5;
        static constexpr double kIntervalMsec = 10.0;
        static constexpr size_t kTxBufferSize = 1024;
    };
} // namespace Task
