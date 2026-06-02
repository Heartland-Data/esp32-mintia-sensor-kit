/**
 * @file command_response.hpp
 * @brief コマンドレスポンスクラスのヘッダ
 * @copyright Copyright (C) 2026, Heartland Data inc. All Rights Reserved.
 * SPDX-FileCopyrightText: 2026 Heartland Data inc.
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once
#include <string>
#include <map>
#include <variant>
#include <memory>

namespace Domain
{
    namespace CommandInterface
    {
        // レスポンスのデータ部はstring, int, double, boolをサポート
        using DataValue = std::variant<std::string, int, double, bool>;
        using DataMap = std::map<std::string, DataValue>;

        // レスポンス結果定義
        inline constexpr const char *kResultOk = "ok";
        inline constexpr const char *kResultError = "error";

        class CommandResponse
        {
        public:
            // 静的生成関数
            static std::unique_ptr<CommandResponse> Create(const std::string &cmdName, const std::string &result, const std::string &message, const DataMap &data = DataMap{});
            // メンバー変数をもとにJSON文字列を返す
            std::string ToJsonString() const;

        private:
            // Private constructor
            CommandResponse(const std::string &cmdName, const std::string &result, const std::string &message, const DataMap &data);
            std::string cmd_;
            std::string result_;
            std::string message_;
            DataMap data_;
        };
    } // namespace CommandInterface
} // namespace Domain
