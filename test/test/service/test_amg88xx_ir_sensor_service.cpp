/**
 * @file test_amg88xx_ir_sensor_service.cpp
 * @brief AMG88xx IR センサーサービス 単体テスト
 * @copyright Copyright (C) 2026, Heartland Data inc. All Rights Reserved.
 * SPDX-FileCopyrightText: 2026 Heartland Data inc.
 * SPDX-License-Identifier: Apache-2.0
 */

#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include "task_sensor.hpp"
#include "amg88xx_ir_sensor_service.hpp"

TEST(Amg88xxIrSensorServiceTest, Run_Basic)
{
    // Given
    Task::ISensorTaskHandler *handler = new Task::TaskSensor(); // モックハンドラーを使用
    service::Amg88xxIrSensorService svc(handler);

    // When
    svc.Run();

    // Then
    SUCCEED();
}
