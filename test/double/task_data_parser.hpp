/**
 * @file task_data_parser.hpp
 * @brief データ解析タスク テストダブル 宣言
 * @copyright Copyright (C) 2026, Heartland Data inc. All Rights Reserved.
 * SPDX-FileCopyrightText: 2026 Heartland Data inc.
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once
#include "idata_parser_task_handler.hpp"

namespace Task
{
    class TaskDataParser : public IDataParserTaskHandler
    {
    public:
        TaskDataParser();
        ~TaskDataParser() override = default;

        bool SendSensorCommand(const Domain::CommandInterface::SensorCommandMsg &cmd) override;
        bool SendResponse(const uint8_t *data, size_t len) override;
    };
} // namespace Task