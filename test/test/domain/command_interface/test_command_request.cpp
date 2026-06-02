/**
 * @file test_command_request.cpp
 * @brief CommandRequestクラスのテストコード
 * @copyright Copyright (C) 2026, Heartland Data inc. All Rights Reserved.
 * SPDX-FileCopyrightText: 2026 Heartland Data inc.
 * SPDX-License-Identifier: Apache-2.0
 */

#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <memory>
#include <variant>
#include "command_interface/command_request.hpp"

using namespace Domain::CommandInterface;

class CommandRequestTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        // テストデータの準備
        params_.clear();
    }

    void TearDown() override
    {
        // テスト後のクリーンアップ
    }

    Params params_;
};

// センサデータ取得開始コマンド(sensor_start)が正常に処理できることを確認
TEST_F(CommandRequestTest, CreateFromJson_SensorStartCommandHandledCorrectly)
{
    // Given
    std::string json = R"({
        "cmd": "sensor_start",
        "params": {}
    })";

    // When
    std::unique_ptr<CommandRequest> req(CommandRequest::CreateFromJson(json));

    // Then
    ASSERT_NE(req, nullptr);
    EXPECT_EQ(req->GetCmd(), "sensor_start");
    const Params &params = req->GetParams();
    EXPECT_TRUE(params.empty());
}

// センサデータ取得停止コマンド(sensor_stop)が正常に処理できることを確認
TEST_F(CommandRequestTest, CreateFromJson_SensorStopCommandHandledCorrectly)
{
    // Given
    std::string json = R"({
        "cmd": "sensor_stop",
        "params": {}
    })";

    // When
    std::unique_ptr<CommandRequest> req(CommandRequest::CreateFromJson(json));

    // Then
    ASSERT_NE(req, nullptr);
    EXPECT_EQ(req->GetCmd(), "sensor_stop");
    const Params &params = req->GetParams();
    EXPECT_TRUE(params.empty());
}

// ファームウェア情報取得コマンド(get_info)が正常に処理できることを確認
TEST_F(CommandRequestTest, CreateFromJson_GetInfoCommandHandledCorrectly)
{
    // Given
    std::string json = R"({
        "cmd": "get_info",
        "params": {}
    })";

    // When
    std::unique_ptr<CommandRequest> req(CommandRequest::CreateFromJson(json));

    // Then
    ASSERT_NE(req, nullptr);
    EXPECT_EQ(req->GetCmd(), "get_info");
    const Params &params = req->GetParams();
    EXPECT_TRUE(params.empty());
}