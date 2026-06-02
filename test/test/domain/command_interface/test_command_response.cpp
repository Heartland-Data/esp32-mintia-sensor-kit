/**
 * @file test_command_response.cpp
 * @brief CommandResponse ドメインオブジェクト 単体テスト
 * @copyright Copyright (C) 2026, Heartland Data inc. All Rights Reserved.
 * SPDX-FileCopyrightText: 2026 Heartland Data inc.
 * SPDX-License-Identifier: Apache-2.0
 */

#include <gtest/gtest.h>
#include "command_interface/command_response.hpp"

using namespace Domain::CommandInterface;

// ファームウェア情報取得のコマンドレスポンスのテスト
TEST(CommandResponseTest, ToJsonString_GetInfoCommandResponse)
{
    DataMap data;
    data["fw_version"] = std::string("1.0.0");
    data["sensor_type"] = std::string("AMG8833");

    auto resp = CommandResponse::Create(
        "get_info",
        "ok",
        "Successfully retrieved device information",
        data);
    ASSERT_NE(resp, nullptr);

    std::string json = resp->ToJsonString();
    std::string expected = R"({"cmd":"get_info","result":"ok","data":{"message":"Successfully retrieved device information","fw_version":"1.0.0","sensor_type":"AMG8833"}})";
    ASSERT_EQ(json, expected);

}

// センサデータ取得開始の成功コマンドレスポンスのテスト
TEST(CommandResponseTest, ToJsonString_SensorStartSuccess)
{
    DataMap data;
    // messageのみ
    auto resp = CommandResponse::Create(
        "sensor_start",
        "ok",
        "Sensor started successfully",
        data);
    ASSERT_NE(resp, nullptr);

    std::string json = resp->ToJsonString();
    std::string expected = R"({"cmd":"sensor_start","result":"ok","data":{"message":"Sensor started successfully"}})";
    ASSERT_EQ(json, expected);

}
