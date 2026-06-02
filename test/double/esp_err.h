/**
 * @file esp_err.h
 * @brief ESP-IDF esp_err.h テストダブル
 * @copyright Copyright (C) 2026, Heartland Data inc. All Rights Reserved.
 * SPDX-FileCopyrightText: 2026 Heartland Data inc.
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#define ESP_OK 0    /*!< esp_err_t value indicating success (no error) */
#define ESP_FAIL -1 /*!< Generic esp_err_t code indicating failure */

typedef int esp_err_t;

inline const char *esp_err_to_name(esp_err_t code)
{
    return "test";
}

#ifdef __cplusplus
}
#endif
