/**
 * @file message_buffer_defs.hpp
 * @brief メッセージバッファとキューの定義
 *
 * @copyright Copyright (C) 2026, Heartland Data inc. All Rights Reserved.
 *
 * SPDX-FileCopyrightText: 2026 Heartland Data inc.
 * SPDX-License-Identifier: Apache-2.0
 */
#pragma once
#include <cstddef>

constexpr size_t kRxMessageBufferSize = 1024; // 受信用メッセージバッファのサイズ
constexpr size_t kTxMessageBufferSize = 16384; // 送信用メッセージバッファのサイズ（MLX90640データ対応）
constexpr size_t kCommandQueueLength = 5; // センサコマンドキューの長さ
