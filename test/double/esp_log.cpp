/**
 * @file esp_log.cpp
 * @brief ESP-IDF esp_log テストダブル 実装
 * @copyright Copyright (C) 2026, Heartland Data inc. All Rights Reserved.
 * SPDX-FileCopyrightText: 2026 Heartland Data inc.
 * SPDX-License-Identifier: Apache-2.0
 */

#include "esp_log.h"
#include <cstdarg>
#include <cstdio>
#include <string>

static std::string g_lastLog;
void ESP_LOGD(const char *tag, const char *fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    char buf[256];
    vsnprintf(buf, sizeof(buf), fmt, args);
    va_end(args);
    g_lastLog = std::string("[D] ") + tag + ": " + buf;
    printf("%s\n", g_lastLog.c_str()); // For demonstration purposes, print to console
}

void ESP_LOGI(const char *tag, const char *fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    char buf[256];
    vsnprintf(buf, sizeof(buf), fmt, args);
    va_end(args);
    g_lastLog = std::string("[I] ") + tag + ": " + buf;
    printf("%s\n", g_lastLog.c_str()); // For demonstration purposes, print to console
}

void ESP_LOGW(const char *tag, const char *fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    char buf[256];
    vsnprintf(buf, sizeof(buf), fmt, args);
    va_end(args);
    g_lastLog = std::string("[W] ") + tag + ": " + buf;
    printf("%s\n", g_lastLog.c_str()); // For demonstration purposes, print to console
}

void ESP_LOGE(const char *tag, const char *fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    char buf[256];
    vsnprintf(buf, sizeof(buf), fmt, args);
    va_end(args);
    g_lastLog = std::string("[E] ") + tag + ": " + buf;
    printf("%s\n", g_lastLog.c_str()); // For demonstration purposes, print to console
}

const char *get_last_log()
{
    return g_lastLog.c_str();
}