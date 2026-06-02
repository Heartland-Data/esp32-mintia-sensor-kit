/**
 * @file command_request.hpp
 * @brief コマンドリクエストクラスのヘッダ
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
        // コマンドのパラメータはstring, int, double, boolなどをサポート
        using ParamValue = std::variant<std::string, int, double, bool>;
        using Params = std::map<std::string, ParamValue>;

        // コマンド名定義
        inline constexpr const char *kCmdSensorStart = "sensor_start";
        inline constexpr const char *kCmdSensorStop = "sensor_stop";
        inline constexpr const char *kCmdGetInfo = "get_info";

        class CommandRequest
        {
        public:
            // Static Create functions
            static std::unique_ptr<CommandRequest> Create(const std::string &cmdName, const Params &params = Params{});
            static std::unique_ptr<CommandRequest> CreateFromJson(const std::string &jsonStr);

            const std::string &GetCmd() const;
            const Params &GetParams() const;
            bool HasParam(const std::string &key) const;
            ParamValue GetParam(const std::string &key) const;

        private:
            // Private constructor
            CommandRequest(const std::string &cmdName, const Params &params);
            std::string cmd_;
            Params params_;
        };
    } // namespace CommandInterface
} // namespace Domain
