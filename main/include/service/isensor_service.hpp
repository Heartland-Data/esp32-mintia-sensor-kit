#pragma once

#include <cstdint>
#include <cstddef>

/**
 * @file isensor_service.hpp
 * @brief センササービスの抽象インターフェース
 * @copyright Copyright (C) 2026, Heartland Data inc. All Rights Reserved.
 * SPDX-FileCopyrightText: 2026 Heartland Data inc.
 * SPDX-License-Identifier: Apache-2.0
 */

namespace service
{
  using SendDataCallback = bool (*)(const uint8_t *data, size_t maxLen, void *userCtx);

  /**
   * @brief センササービスのインターフェース
   */
  class ISensorService
  {
  public:
    virtual ~ISensorService() = default;

    /**
     * @brief サービスの初期化
     * @return true: 初期化成功, false: 失敗
     */
    virtual bool Initialize() = 0;

    /**
     * @brief センサデータ取得・処理の実行
     */
    virtual void Run() = 0;
  };

} // namespace service
