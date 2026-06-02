#pragma once

/**
 * @file amg88xx_ir_sensor_service.hpp
 * @brief AMG88xx用IRセンササービス宣言
 * @copyright Copyright (C) 2026, Heartland Data inc. All Rights Reserved.
 * SPDX-FileCopyrightText: 2026 Heartland Data inc.
 * SPDX-License-Identifier: Apache-2.0
 */

#include "isensor_service.hpp"
#include "amg88xx_esp32.h"
#include "driver/i2c_master.h"
#include "isensor_task_handler.hpp"

namespace service
{

  /**
   * @brief AMG88xx用IRセンササービス
   */
  class Amg88xxIrSensorService : public ISensorService
  {
  public:
    Amg88xxIrSensorService(Task::ISensorTaskHandler *handler);
    ~Amg88xxIrSensorService() override;

    bool Initialize() override;
    void Run() override;

  private:
    Task::ISensorTaskHandler *handler_;
    bool InitializeI2c();
    bool InitializeAmg88xx();

    void SendAmg88xxSensorData(const float *pixels, size_t pixelCount, float thermistorTemp);

    amg88xx_handle_t amgHandle_;
    bool i2cInitialized_;
    bool sensorInitialized_;
    i2c_master_bus_handle_t busHandle_;
    i2c_master_dev_handle_t devHandle_;
  };

} // namespace service
