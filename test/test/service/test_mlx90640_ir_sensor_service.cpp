/**
 * @file test_mlx90640_ir_sensor_service.cpp
 * @brief MLX90640 IR センサーサービス 単体テスト
 * @copyright Copyright (C) 2026, Heartland Data inc. All Rights Reserved.
 * SPDX-FileCopyrightText: 2026 Heartland Data inc.
 * SPDX-License-Identifier: Apache-2.0
 */

#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include "task_sensor.hpp"
#include "mlx90640_ir_sensor_service.hpp"

TEST(Mlx90640IrSensorServiceTest, Run_Basic)
{
    // Given
    Task::ISensorTaskHandler *handler = new Task::TaskSensor(); // モックハンドラーを使用
    service::Mlx90640IrSensorService svc(handler);

    // When
    svc.Run();

    // Then
    SUCCEED();
}
