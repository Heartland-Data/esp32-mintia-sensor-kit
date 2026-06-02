/**
 * @file isensor_task_handler.hpp
 * @brief センサタスクハンドラ インターフェース テストダブル
 * @copyright Copyright (C) 2026, Heartland Data inc. All Rights Reserved.
 * SPDX-FileCopyrightText: 2026 Heartland Data inc.
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once
#include <cstdint>
#include <cstddef>

namespace Task
{
    class ISensorTaskHandler
    {
    public:
        virtual ~ISensorTaskHandler() = default;

        virtual bool SendSensorData(const uint8_t *data, size_t len) = 0;
        virtual void DelayMs(uint32_t ms) = 0;
    };
}
