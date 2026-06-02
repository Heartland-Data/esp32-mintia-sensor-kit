/**
 * @file xm125_sensor_service.hpp
 * @brief XM125（存在検知）用センササービス宣言
 * @copyright Copyright (C) 2026, Heartland Data inc. All Rights Reserved.
 * SPDX-FileCopyrightText: 2026 Heartland Data inc.
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include "isensor_service.hpp"
#include "xm125_esp32.h"
#include "driver/i2c_master.h"
#include "isensor_task_handler.hpp"

namespace service
{
    /**
     * @brief XM125用センササービス
     */
    class Xm125SensorService : public ISensorService
    {
    public:
        Xm125SensorService(Task::ISensorTaskHandler *handler);
        ~Xm125SensorService() override;

        bool Initialize() override;
        void Run() override;

    private:
        Task::ISensorTaskHandler *handler_;
        bool InitializeI2c();
        bool InitializeXm125();
        void SendXm125SensorData(const xm125_presence_result_t *result);

        xm125_handle_t xm125Handle_;
        bool i2cInitialized_;
        bool sensorInitialized_;
        i2c_master_bus_handle_t busHandle_;
        i2c_master_dev_handle_t devHandle_;
    };
}
