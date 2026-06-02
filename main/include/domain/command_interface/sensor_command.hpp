#pragma once

/**
 * @file sensor_command.hpp
 * @brief センサコマンド定義
 * @copyright Copyright (C) 2026, Heartland Data inc. All Rights Reserved.
 * SPDX-FileCopyrightText: 2026 Heartland Data inc.
 * SPDX-License-Identifier: Apache-2.0
 */

#include <cstdint>

namespace Domain
{
    namespace CommandInterface
    {
        /**
         * @brief センサコマンド種別
         */
        enum class SensorCommandType : uint32_t
        {
            kStart = 0,     ///< センサ開始
            kStop = 1,      ///< センサ停止
            kSetParam = 2, ///< パラメータ設定（将来拡張用）
        };

        /**
         * @brief センサコマンドメッセージ
         */
        struct SensorCommandMsg
        {
            SensorCommandType cmd; ///< コマンド種別
            uint32_t param;          ///< パラメータ（将来拡張用）

            SensorCommandMsg() : cmd(SensorCommandType::kStop), param(0) {}
            SensorCommandMsg(SensorCommandType command, uint32_t parameter = 0)
                : cmd(command), param(parameter) {}
        };

    } // namespace CommandInterface
} // namespace Domain
