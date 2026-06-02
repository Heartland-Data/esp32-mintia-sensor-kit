/**
 * @file sensor_data_message.cpp
 * @brief センサデータメッセージクラスの実装
 * @copyright Copyright (C) 2026, Heartland Data inc. All Rights Reserved.
 * SPDX-FileCopyrightText: 2026 Heartland Data inc.
 * SPDX-License-Identifier: Apache-2.0
 */

#include <cJSON.h>
#include "command_interface/sensor_data_message.hpp"

namespace Domain
{
    namespace CommandInterface
    {
        /**
         * @brief SensorDataMessageインスタンスを生成するファクトリメソッド
         * @param sensorType センサ種別名
         * @param data センサデータのマップ
         * @return SensorDataMessageインスタンスのポインタ（失敗時はnullptr）
         */
        std::unique_ptr<SensorDataMessage> SensorDataMessage::Create(const std::string& sensorType, const SensorDataMap& data)
        {
            if (sensorType.empty())
            {
                return nullptr;
            }
            return std::unique_ptr<SensorDataMessage>(new SensorDataMessage(sensorType, data));
        }

        /**
         * @brief SensorDataMessageのコンストラクタ
         * @param sensorType センサ種別名
         * @param data センサデータのマップ
         */
        SensorDataMessage::SensorDataMessage(const std::string& sensorType, const SensorDataMap& data)
            : sensor_type_(sensorType), data_(data) {}

        /**
         * @brief センサデータをJSON文字列に変換する
         * @return JSON形式の文字列
         */
        std::string SensorDataMessage::ToJsonString() const
        {
            cJSON* root = cJSON_CreateObject();
            cJSON_AddStringToObject(root, "sensor_type", sensor_type_.c_str());

            cJSON* dataObj = cJSON_CreateObject();

            for (const auto& kv : data_)
            {
                const auto& key = kv.first;
                const auto& val = kv.second;

                if (std::holds_alternative<std::string>(val))
                {
                    cJSON_AddStringToObject(dataObj, key.c_str(), std::get<std::string>(val).c_str());
                }
                else if (std::holds_alternative<int>(val))
                {
                    cJSON_AddNumberToObject(dataObj, key.c_str(), std::get<int>(val));
                }
                else if (std::holds_alternative<double>(val))
                {
                    cJSON_AddNumberToObject(dataObj, key.c_str(), std::get<double>(val));
                }
                else if (std::holds_alternative<bool>(val))
                {
                    cJSON_AddBoolToObject(dataObj, key.c_str(), std::get<bool>(val));
                }
                else if (std::holds_alternative<std::vector<double>>(val))
                {
                    const auto& vec = std::get<std::vector<double>>(val);
                    cJSON* arr = cJSON_CreateDoubleArray(vec.data(), static_cast<int>(vec.size()));
                    cJSON_AddItemToObject(dataObj, key.c_str(), arr);
                }
                else if (std::holds_alternative<std::vector<std::vector<double>>>(val))
                {
                    const auto& mat = std::get<std::vector<std::vector<double>>>(val);
                    cJSON* arr2d = cJSON_CreateArray();
                    for (const auto& row : mat)
                    {
                        cJSON* arrRow = cJSON_CreateDoubleArray(row.data(), static_cast<int>(row.size()));
                        cJSON_AddItemToArray(arr2d, arrRow);
                    }
                    cJSON_AddItemToObject(dataObj, key.c_str(), arr2d);
                }
            }

            cJSON_AddItemToObject(root, "data", dataObj);

            char* jsonStr = cJSON_PrintUnformatted(root);
            std::string resultStr = jsonStr ? jsonStr : "";
            if (jsonStr)
                cJSON_free(jsonStr);
            cJSON_Delete(root);
            return resultStr;
        }
    } // namespace CommandInterface
} // namespace Domain
