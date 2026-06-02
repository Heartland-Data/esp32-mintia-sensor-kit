/**
 * @file task_sensor.hpp
 * @brief センサタスク テストダブル 宣言
 * @copyright Copyright (C) 2026, Heartland Data inc. All Rights Reserved.
 * SPDX-FileCopyrightText: 2026 Heartland Data inc.
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once
#include "isensor_task_handler.hpp"

namespace Task
{
    class TaskSensor : public ISensorTaskHandler
    {
    public:
        TaskSensor();
        ~TaskSensor() override = default;

        bool SendSensorData(const uint8_t *data, size_t len) override;
        void DelayMs(uint32_t ms) override;
    };
} // namespace Task
