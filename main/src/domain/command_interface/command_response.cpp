/**
 * @file command_response.cpp
 * @brief コマンドレスポンスクラスの実装
 * @copyright Copyright (C) 2026, Heartland Data inc. All Rights Reserved.
 * SPDX-FileCopyrightText: 2026 Heartland Data inc.
 * SPDX-License-Identifier: Apache-2.0
 */

#include <cJSON.h>
#include "command_interface/command_response.hpp"

namespace Domain
{
    namespace CommandInterface
    {
        /**
         * @brief コンストラクタ
         * @param cmdName コマンド名
         * @param result 結果("ok"または"error")
         * @param message 詳細メッセージ
         * @param data 追加データ
         */
        CommandResponse::CommandResponse(const std::string &cmdName, const std::string &result, const std::string &message, const DataMap &data)
            : cmd_(cmdName), result_(result), message_(message), data_(data) {}

        /**
         * @brief コマンドレスポンスを生成する静的関数
         * @param cmdName コマンド名
         * @param result 結果("ok"または"error")
         * @param message 詳細メッセージ
         * @param data 追加データ
         * @retval CommandResponse* 生成されたインスタンス（失敗時はnullptr）
         */
        std::unique_ptr<CommandResponse> CommandResponse::Create(const std::string &cmdName, const std::string &result, const std::string &message, const DataMap &data)
        {
            if (cmdName.empty() || result.empty() || message.empty())
            {
                return nullptr;
            }
            return std::unique_ptr<CommandResponse>(new CommandResponse(cmdName, result, message, data));
        }

        /**
         * @brief メンバー変数をもとにJSON文字列を生成する
         * @return JSON形式の文字列
         */
        std::string CommandResponse::ToJsonString() const
        {
            cJSON *root = cJSON_CreateObject();
            cJSON_AddStringToObject(root, "cmd", cmd_.c_str());
            cJSON_AddStringToObject(root, "result", result_.c_str());

            cJSON *dataObj = cJSON_CreateObject();
            cJSON_AddStringToObject(dataObj, "message", message_.c_str());

            for (const auto &kv : data_)
            {
                if (kv.first == "message")
                    continue; // messageは専用
                const DataValue &val = kv.second;
                if (std::holds_alternative<std::string>(val))
                {
                    cJSON_AddStringToObject(dataObj, kv.first.c_str(), std::get<std::string>(val).c_str());
                }
                else if (std::holds_alternative<int>(val))
                {
                    cJSON_AddNumberToObject(dataObj, kv.first.c_str(), std::get<int>(val));
                }
                else if (std::holds_alternative<double>(val))
                {
                    cJSON_AddNumberToObject(dataObj, kv.first.c_str(), std::get<double>(val));
                }
                else if (std::holds_alternative<bool>(val))
                {
                    cJSON_AddBoolToObject(dataObj, kv.first.c_str(), std::get<bool>(val));
                }
            }
            cJSON_AddItemToObject(root, "data", dataObj);

            char *jsonStr = cJSON_PrintUnformatted(root);
            std::string resultStr = jsonStr ? jsonStr : "";
            if (jsonStr)
                cJSON_free(jsonStr);
            cJSON_Delete(root);
            return resultStr;
        }

    } // namespace CommandInterface
} // namespace Domain
