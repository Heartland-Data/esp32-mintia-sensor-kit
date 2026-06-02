/**
 * @file icommunication_task_handler.hpp
 * @brief 通信タスク用ハンドラーIF
 *
 * @copyright Copyright (C) 2026, Heartland Data inc. All Rights Reserved.
 *
 * SPDX-FileCopyrightText: 2026 Heartland Data inc.
 * SPDX-License-Identifier: Apache-2.0
 */
#pragma once
#include <cstdint>
#include <cstddef>

namespace Task
{
    class ICommunicationTaskHandler
    {
    public:
        virtual ~ICommunicationTaskHandler() = default;

        virtual void OnReceive(const uint8_t *data, size_t len) = 0;
        virtual const uint8_t *GetTxMessageCache() const = 0;
        virtual size_t GetTxMessageCacheLen() const = 0;
    };
}
