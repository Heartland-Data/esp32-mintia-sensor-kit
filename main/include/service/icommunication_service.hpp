/**
 * @file icommunication_service.hpp
 * @brief インターフェース通信サービスの抽象インターフェース
 *
 * @copyright Copyright (C) 2026, Heartland Data inc. All Rights Reserved.
 *
 * SPDX-FileCopyrightText: 2026 Heartland Data inc.
 * SPDX-License-Identifier: Apache-2.0
 */
#pragma once
#include <cstdint>
#include <cstddef>
namespace service
{
    /**
     * @brief インターフェース通信サービスの抽象インターフェース
     */
    class ICommunicationService
    {
    public:
        virtual ~ICommunicationService() = default;

        /**
         * @brief サービスの初期化
         * @return true: 初期化成功, false: 失敗
         */
        virtual bool Initialize() = 0;

        /**
         * @brief サービスの実行
         */
        virtual void Run() = 0;
    };

} // namespace service
