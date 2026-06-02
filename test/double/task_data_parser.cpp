/**
 * @file task_data_parser.cpp
 * @brief データ解析タスク テストダブル 実装
 * @copyright Copyright (C) 2026, Heartland Data inc. All Rights Reserved.
 * SPDX-FileCopyrightText: 2026 Heartland Data inc.
 * SPDX-License-Identifier: Apache-2.0
 */

#include "task_data_parser.hpp"
#include "esp_log.h"

namespace Task
{
    TaskDataParser::TaskDataParser()
    {
    }

    bool TaskDataParser::SendSensorCommand(const Domain::CommandInterface::SensorCommandMsg &cmd)
    {
        ESP_LOGI("TaskDataParser", "Sending sensor command: cmd=%d, param=%lu",
                 static_cast<int>(cmd.cmd), cmd.param);
        return true;
    }

    bool TaskDataParser::SendResponse(const uint8_t *data, size_t len)
    {
        ESP_LOGI("TaskDataParser", "Sending response(%d): %.*s", static_cast<int>(len), static_cast<int>(len), reinterpret_cast<const char *>(data));
        return true;
    }

} // namespace Task