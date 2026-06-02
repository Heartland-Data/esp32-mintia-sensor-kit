/**
 * @file command_request.cpp
 * @brief コマンドリクエストクラスの実装
 * @copyright Copyright (C) 2026, Heartland Data inc. All Rights Reserved.
 * SPDX-FileCopyrightText: 2026 Heartland Data inc.
 * SPDX-License-Identifier: Apache-2.0
 */

#include <cJSON.h>
#include "command_interface/command_request.hpp"

namespace Domain
{
    namespace CommandInterface
    {
        /**
         * @brief コンストラクタ
         * @param cmdName コマンド名
         * @param params パラメータマップ
         */
        CommandRequest::CommandRequest(const std::string &cmdName, const Params &params)
            : cmd_(cmdName), params_(params) {}

        /**
         * @brief コマンド名とパラメータからCommandRequestを生成
         * @param cmdName コマンド名
         * @param params パラメータマップ
         * @return CommandRequestのポインタ（生成失敗時はnullptr）
         */
        std::unique_ptr<CommandRequest> CommandRequest::Create(const std::string &cmdName, const Params &params)
        {
            if (cmdName.empty())
            {
                return nullptr;
            }

            return std::unique_ptr<CommandRequest>(new CommandRequest(cmdName, params));
        }

        /**
         * @brief JSON文字列からCommandRequestを生成
         * @param jsonStr JSON形式の文字列
         * @return CommandRequestのポインタ（生成失敗時はnullptr）
         */
        std::unique_ptr<CommandRequest> CommandRequest::CreateFromJson(const std::string &jsonStr)
        {
            cJSON *root = cJSON_Parse(jsonStr.c_str());
            if (!root)
            {
                return nullptr;
            }

            cJSON *cmdItem = cJSON_GetObjectItem(root, "cmd");
            if (!cmdItem || !cJSON_IsString(cmdItem))
            {
                cJSON_Delete(root);
                return nullptr;
            }

            std::string cmdName = cmdItem->valuestring;
            Params params;

            cJSON *paramsItem = cJSON_GetObjectItem(root, "params");
            if (paramsItem && cJSON_IsObject(paramsItem))
            {
                cJSON *entry = nullptr;
                cJSON_ArrayForEach(entry, paramsItem)
                {
                    if (cJSON_IsString(entry))
                    {
                        params[entry->string] = std::string(entry->valuestring);
                    }
                    else if (cJSON_IsNumber(entry))
                    {
                        // 整数か浮動小数点かを判定
                        double dval = entry->valuedouble;
                        int ival = entry->valueint;
                        if (dval == static_cast<double>(ival))
                        {
                            params[entry->string] = ival;
                        }
                        else
                        {
                            params[entry->string] = dval;
                        }
                    }
                    else if (cJSON_IsBool(entry))
                    {
                        params[entry->string] = cJSON_IsTrue(entry);
                    }
                }
            }
            cJSON_Delete(root);

            return std::unique_ptr<CommandRequest>(new CommandRequest(cmdName, params));
        }

        /**
         * @brief コマンド名を取得
         * @return コマンド名
         */
        const std::string &CommandRequest::GetCmd() const
        {
            return cmd_;
        }

        /**
         * @brief パラメータマップを取得
         * @return パラメータマップ
         */
        const Params &CommandRequest::GetParams() const
        {
            return params_;
        }

        /**
         * @brief 指定キーのパラメータ有無を判定
         * @param key パラメータキー
         * @retval true 存在する
         * @retval false 存在しない
         */
        bool CommandRequest::HasParam(const std::string &key) const
        {
            return params_.find(key) != params_.end();
        }

        /**
         * @brief 指定キーのパラメータ値を取得
         * @param key パラメータキー
         * @return パラメータ値（見つからない場合は空文字列）
         */
        ParamValue CommandRequest::GetParam(const std::string &key) const
        {
            auto it = params_.find(key);
            if (it != params_.end())
            {
                return it->second;
            }

            // パラメータが見つからない場合は空の値を返す
            static const ParamValue dummy = std::string("");
            return dummy;
        }

    } // namespace CommandInterface
} // namespace Domain
