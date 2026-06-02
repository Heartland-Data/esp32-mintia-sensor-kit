/**
 * @file idata_parser_task_handler.hpp
 * @brief データ解析タスクハンドラ インターフェース テストダブル
 * @copyright Copyright (C) 2026, Heartland Data inc. All Rights Reserved.
 * SPDX-FileCopyrightText: 2026 Heartland Data inc.
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <cstdint>
#include <cstddef>
#include "command_interface/sensor_command.hpp"

namespace Task
{
    class IDataParserTaskHandler
    {
    public:
        virtual ~IDataParserTaskHandler() = default;

        virtual bool SendSensorCommand(const Domain::CommandInterface::SensorCommandMsg &cmd) = 0;
        virtual bool SendResponse(const uint8_t *data, size_t len) = 0;
    };
}
