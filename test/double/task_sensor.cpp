/**
 * @file task_sensor.cpp
 * @brief センサタスク テストダブル 実装
 * @copyright Copyright (C) 2026, Heartland Data inc. All Rights Reserved.
 * SPDX-FileCopyrightText: 2026 Heartland Data inc.
 * SPDX-License-Identifier: Apache-2.0
 */

#include "task_sensor.hpp"
#include "esp_log.h"

namespace Task
{

    TaskSensor::TaskSensor()
    {
        ESP_LOGI("TaskSensor", "TaskSensor initialized");
    }

    bool TaskSensor::SendSensorData(const uint8_t *data, size_t len)
    {
        ESP_LOGI("TaskSensor", "Sending sensor data(len=%d): %.*s", static_cast<int>(len), static_cast<int>(len), reinterpret_cast<const char *>(data));
        return true;
    }

    void TaskSensor::DelayMs(uint32_t ms)
    {
        ESP_LOGI("TaskSensor", "Delay requested: %lu ms (mocked - not actually delaying)", ms);
        // テスト環境では実際に遅延させない
    }

} // namespace Task
