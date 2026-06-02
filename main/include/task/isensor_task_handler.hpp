/**
 * @file isensor_task_handler.hpp
 * @brief IRセンサタスク用ハンドラーIF
 *
 * @copyright Copyright (C) 2026, Heartland Data inc. All Rights Reserved.
 *
 * SPDX-FileCopyrightText: 2026 Heartland Data inc.
 * SPDX-License-Identifier: Apache-2.0
 */
#pragma once
#include <cstdint>
#include <cstddef>

namespace Task
{
    class ISensorTaskHandler
    {
    public:
        virtual ~ISensorTaskHandler() = default;

        /**
         * @brief センサーデータ送信
         * @param data 送信データ
         * @param len データ長
         * @return true: 送信成功, false: 送信失敗
         */
        virtual bool SendSensorData(const uint8_t *data, size_t len) = 0;

        /**
         * @brief 遅延処理
         * @param ms 遅延時間（ミリ秒）
         */
        virtual void DelayMs(uint32_t ms) = 0;
    };
}
