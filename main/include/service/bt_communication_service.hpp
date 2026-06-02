/**
 * @file bt_communication_service.hpp
 * @brief Bluetooth SPP通信サービスのヘッダ
 *
 * @copyright Copyright (C) 2026, Heartland Data inc. All Rights Reserved.
 *
 * SPDX-FileCopyrightText: 2026 Heartland Data inc.
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <cstdint>
#include <cstddef>
#include "task/icommunication_task_handler.hpp"
#include "service/icommunication_service.hpp"

#ifdef __cplusplus
extern "C"
{
#endif

#include "esp_bt.h"
#include "esp_bt_main.h"
#include "esp_gap_bt_api.h"
#include "esp_spp_api.h"

#ifdef __cplusplus
}
#endif

namespace service
{
    class BtCommunicationService : public ICommunicationService
    {
    public:
        explicit BtCommunicationService(Task::ICommunicationTaskHandler *handler);
        ~BtCommunicationService() override;

        bool Initialize() override;
        void Run() override;

    private:
        Task::ICommunicationTaskHandler *handler_;
        bool sppConnected_;  // SPP接続状態管理用
        uint32_t sppHandle_; // SPP送信用ハンドル

        // シングルトンインスタンス
        static BtCommunicationService *instance_;

        void ProcessSendData();
        static void SppCallback(esp_spp_cb_event_t event, esp_spp_cb_param_t *param);
        static void GapCallback(esp_bt_gap_cb_event_t event, esp_bt_gap_cb_param_t *param);
        static char *BluetoothDevAddr2str(uint8_t *bda, char *str, size_t size);
    };
} // namespace service
