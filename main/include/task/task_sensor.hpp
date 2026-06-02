#pragma once

/**
 * @file task_sensor.hpp
 * @brief センサタスク（サービス抽象化）
 * @copyright Copyright (C) 2026, Heartland Data inc. All Rights Reserved.
 * SPDX-FileCopyrightText: 2026 Heartland Data inc.
 * SPDX-License-Identifier: Apache-2.0
 */

#include "abstract_task.hpp"
#include "service/isensor_service.hpp"
#include "freertos/queue.h"
#include "freertos/message_buffer.h"
#include "domain/command_interface/sensor_command.hpp"
#include "task/isensor_task_handler.hpp"

namespace Task
{

  /**
   * @brief センサタスク（サービス抽象化）
   */
  class SensorTask : public Task::AbstractTask, public ISensorTaskHandler
  {
  public:
    explicit SensorTask(QueueHandle_t commandQueue, MessageBufferHandle_t txBuffer);
    ~SensorTask() override;
    void Run() override;

    // ISensorTaskHandlerの実装
    bool SendSensorData(const uint8_t *data, size_t len) override;
    void DelayMs(uint32_t ms) override;

  private:
    service::ISensorService *service_;
    QueueHandle_t commandQueue_;
    MessageBufferHandle_t txBuffer_;
    bool isSensorRunning_ = false;

    void ProcessCommand();

    static constexpr const char *kName = "sensor_task";
    static constexpr uint32_t kStackDepth = 8192;
    static constexpr int kPriority = 1;
    static constexpr double kIntervalMsec = 500.0;
  };

} // namespace task
