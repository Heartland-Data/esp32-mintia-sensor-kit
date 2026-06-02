/**
 * @file main.cpp
 * @brief ESP32 ファームウェア エントリーポイント
 * @copyright Copyright (C) 2026, Heartland Data inc. All Rights Reserved.
 * SPDX-FileCopyrightText: 2026 Heartland Data inc.
 * SPDX-License-Identifier: Apache-2.0
 */

#include <vector>
#include <memory>
#include "uart_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "abstract_task.hpp"
#include "task_sensor.hpp"
#include "task_communication.hpp"
#include "task_data_parser.hpp"
#include "freertos/queue.h"
#include "domain/command_interface/sensor_command.hpp"
#include "task/message_buffer_defs.hpp"
// DT_INCLUDE

std::vector<std::shared_ptr<Task::AbstractTask>> tasks;

/**
 * @brief FreeRTOSタスクの生成
 *
 */
void CreateTask()
{
  // メッセージバッファとキューの生成
  MessageBufferHandle_t rxMessageBuffer = xMessageBufferCreate(kRxMessageBufferSize);
  MessageBufferHandle_t txMessageBuffer = xMessageBufferCreate(kTxMessageBufferSize);
  QueueHandle_t sensorCommandQueue = xQueueCreate(kCommandQueueLength, sizeof(Domain::CommandInterface::SensorCommandMsg));

  // DataParserService/TaskDataParser生成
  tasks.emplace_back(std::make_shared<Task::TaskDataParser>(rxMessageBuffer, txMessageBuffer, sensorCommandQueue));

  // UART通信サービス生成
  tasks.emplace_back(std::make_shared<Task::TaskCommunication>(rxMessageBuffer, txMessageBuffer));

  // AMG88xx センササービスとタスクを生成
  tasks.emplace_back(std::make_shared<Task::SensorTask>(sensorCommandQueue, txMessageBuffer));

  for (auto &task : tasks)
  {
    task->CreateTask(0); // コア0でタスク生成
  }
}

extern "C" void app_main(void)
{
  // UART2を初期化してログ出力先を変更
  init_uart2_for_log();

  CreateTask();
}
