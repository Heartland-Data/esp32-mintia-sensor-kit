/**
 * @file test_sensor_data_message.cpp
 * @brief SensorDataMessage ドメインオブジェクト 単体テスト
 * @copyright Copyright (C) 2026, Heartland Data inc. All Rights Reserved.
 * SPDX-FileCopyrightText: 2026 Heartland Data inc.
 * SPDX-License-Identifier: Apache-2.0
 */

#include <gtest/gtest.h>
#include <cJSON.h>
#include "command_interface/sensor_data_message.hpp"

using namespace Domain::CommandInterface;

// AMG8833センサの温度データを含むセンサデータメッセージのテスト
TEST(SensorDataMessageTest, ToJsonString_AMG8833TemperatureData)
{
    // 8x8温度配列
    std::vector<std::vector<double>> temperature = {
        {25.5, 26.0, 26.5, 27.0, 27.5, 28.0, 28.5, 29.0},
        {25.0, 25.5, 26.0, 26.5, 27.0, 27.5, 28.0, 28.5},
        {24.5, 25.0, 25.5, 26.0, 26.5, 27.0, 27.5, 28.0},
        {24.0, 24.5, 25.0, 25.5, 26.0, 26.5, 27.0, 27.5},
        {23.5, 24.0, 24.5, 25.0, 25.5, 26.0, 26.5, 27.0},
        {23.0, 23.5, 24.0, 24.5, 25.0, 25.5, 26.0, 26.5},
        {22.5, 23.0, 23.5, 24.0, 24.5, 25.0, 25.5, 26.0},
        {22.0, 22.5, 23.0, 23.5, 24.0, 24.5, 25.0, 25.5}
    };

    SensorDataMap data;
    data["temperature"] = temperature;
    data["ambient"] = 24.5;

    auto msg = SensorDataMessage::Create("AMG8833", data);
    ASSERT_NE(msg, nullptr);

    std::string json = msg->ToJsonString();

    std::string expected = R"({"sensor_type":"AMG8833","data":{"temperature":[[25.5,26,26.5,27,27.5,28,28.5,29],[25,25.5,26,26.5,27,27.5,28,28.5],[24.5,25,25.5,26,26.5,27,27.5,28],[24,24.5,25,25.5,26,26.5,27,27.5],[23.5,24,24.5,25,25.5,26,26.5,27],[23,23.5,24,24.5,25,25.5,26,26.5],[22.5,23,23.5,24,24.5,25,25.5,26],[22,22.5,23,23.5,24,24.5,25,25.5]],"ambient":24.5}})";

    cJSON* actual = cJSON_Parse(json.c_str());
    cJSON* expect = cJSON_Parse(expected.c_str());
    ASSERT_NE(actual, nullptr);
    ASSERT_NE(expect, nullptr);
    ASSERT_TRUE(cJSON_Compare(actual, expect, 1));
    cJSON_Delete(actual);
    cJSON_Delete(expect);

}
