/**
 * @file abstract_task.cpp
 * @brief FreeRTOS タスク抽象基底クラス 実装
 * @copyright Copyright (C) 2026, Heartland Data inc. All Rights Reserved.
 * SPDX-FileCopyrightText: 2026 Heartland Data inc.
 * SPDX-License-Identifier: Apache-2.0
 */

#include <string>
#include <sstream>
#include <iomanip>
#include "esp_log.h"
#include "abstract_task.hpp"
// DT_INCLUDE

namespace Task
{
    AbstractTask::AbstractTask(const char *taskName, const uint32_t stackDepth, int priority, const double intervalMsec)
        : kName(taskName),
          kStackDepth(stackDepth),
          kPriority(priority),
          kIntervalMsec(intervalMsec)
    {
        std::ostringstream oss;
        oss << "Name = " << kName
            << ", Stack Depth = " << (unsigned long)kStackDepth
            << ", Priority = " << kPriority
            << ", Interval(msec) = " << std::fixed << std::setprecision(2) << kIntervalMsec;
        ESP_LOGI(kTag, "%s", oss.str().c_str());
    }

    AbstractTask::~AbstractTask()
    {
        DeleteTask();
    }

    /**
     * @brief Task生成
     *
     * @param core コア番号
     */
    void AbstractTask::CreateTask(int core)
    {
        xTaskCreatePinnedToCore(TaskEntryPoint,
                                kName,
                                kStackDepth,
                                this,
                                kPriority,
                                &handle_,
                                core);
    }

    /**
     * @brief Task削除
     *
     */
    void AbstractTask::DeleteTask()
    {
        if (handle_ != nullptr)
        {
            vTaskDelete(handle_);
            handle_ = nullptr;
        }
    }

    /**
     * @brief タスク処理の関数
     * @note FreeRTOSではstaticの関数を渡す必要がある
     * [C++でFreeRTOSのタスクをいい感じにつくる](http://idken.net/posts/2017-02-01-freertos_task_cpp/)
     */
    void AbstractTask::TaskEntryPoint(void *taskInstance)
    {
        static_cast<AbstractTask *>(taskInstance)->Run();
    }
}
