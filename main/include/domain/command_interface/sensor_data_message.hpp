/**
 * @file sensor_data_message.hpp
 * @brief センサデータメッセージクラスのヘッダ
 * @copyright Copyright (C) 2026, Heartland Data inc. All Rights Reserved.
 * SPDX-FileCopyrightText: 2026 Heartland Data inc.
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once
#include <string>
#include <map>
#include <variant>
#include <vector>
#include <memory>

namespace Domain
{
    namespace CommandInterface
    {
        // センサデータ値型
        using SensorDataValue = std::variant<
            std::string,
            int,
            double,
            bool,
            std::vector<double>,
            std::vector<std::vector<double>>>;
        using SensorDataMap = std::map<std::string, SensorDataValue>;

        class SensorDataMessage
        {
        public:
            static std::unique_ptr<SensorDataMessage> Create(const std::string &sensorType, const SensorDataMap &data);

            std::string ToJsonString() const;

        private:
            // Private constructor
            SensorDataMessage(const std::string &sensorType, const SensorDataMap &data);
            std::string sensor_type_;
            SensorDataMap data_;
        };
    } // namespace CommandInterface
} // namespace Domain
