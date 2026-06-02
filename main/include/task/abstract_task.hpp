/**
 * @file abstract_task.hpp
 * @brief FreeRTOS タスク抽象基底クラス 定義
 * @copyright Copyright (C) 2026, Heartland Data inc. All Rights Reserved.
 * SPDX-FileCopyrightText: 2026 Heartland Data inc.
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

namespace Task
{
    /**
     * @brief Task用インターフェースクラス
     *
     */
    class AbstractTask
    {
    public:
        // デフォルトコンストラクタを禁止
        AbstractTask() = delete;
        AbstractTask(const char *taskName, const uint32_t stackDepth, int priority, const double intervalMsec);
        virtual ~AbstractTask();
        void CreateTask(int core);
        void DeleteTask();
        virtual void Run() = 0;
        static void TaskEntryPoint(void *taskInstance);

    protected:
        TaskHandle_t handle_ = nullptr;
        const char *kName;
        const uint32_t kStackDepth;
        const int kPriority;
        const double kIntervalMsec;
        static constexpr const char *kTag = "AbstractTask";
    };
}
