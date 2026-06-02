/**
 * @file task_data_parser.hpp
 * @brief データ解析タスク クラス定義
 * @copyright Copyright (C) 2026, Heartland Data inc. All Rights Reserved.
 * SPDX-FileCopyrightText: 2026 Heartland Data inc.
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once
#include "abstract_task.hpp"
#include "freertos/message_buffer.h"
#include "freertos/queue.h"
#include "service/data_parser_service.hpp"
#include "domain/command_interface/sensor_command.hpp"
#include "task/message_buffer_defs.hpp"
#include "task/idata_parser_task_handler.hpp"

namespace Task
{
    class TaskDataParser : public AbstractTask, public IDataParserTaskHandler
    {
    public:
        TaskDataParser(MessageBufferHandle_t rxBuffer, MessageBufferHandle_t txBuffer, QueueHandle_t sensorQueue);
        void Run() override;

        // センサへコマンド送信
        bool SendSensorCommand(const Domain::CommandInterface::SensorCommandMsg &cmd) override;

        // IDataParserTaskHandlerの実装
        bool SendResponse(const uint8_t *data, size_t len) override;

    private:
        MessageBufferHandle_t rxBuffer_;
        MessageBufferHandle_t txBuffer_;
        QueueHandle_t SensorQueue_;
        service::IDataParserService *parserService_;
        static constexpr const char *const kName = "task_data_parser";
        static constexpr uint32_t kStackDepth = 4096;
        static constexpr int kPriority = 3;
        static constexpr double kIntervalMsec = 10.0;
    };
} // namespace Task
