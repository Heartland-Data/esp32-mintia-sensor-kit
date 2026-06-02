/**
 * @file esp_log.h
 * @brief ESP-IDF esp_log テストダブル 宣言
 * @copyright Copyright (C) 2026, Heartland Data inc. All Rights Reserved.
 * SPDX-FileCopyrightText: 2026 Heartland Data inc.
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once
#include <string>

#ifdef __cplusplus
extern "C"
{
#endif

    void ESP_LOGD(const char *tag, const char *fmt, ...);
    void ESP_LOGI(const char *tag, const char *fmt, ...);
    void ESP_LOGW(const char *tag, const char *fmt, ...);
    void ESP_LOGE(const char *tag, const char *fmt, ...);

    // ログ内容取得用
    const char *get_last_log();

#ifdef __cplusplus
}
#endif
