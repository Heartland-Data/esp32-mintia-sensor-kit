/**
 * @file bt_communication_service.cpp
 * @brief Bluetooth SPP通信サービスの実装
 *
 * @copyright Copyright (C) 2026, Heartland Data inc. All Rights Reserved.
 *
 * SPDX-FileCopyrightText: 2026 Heartland Data inc.
 * SPDX-License-Identifier: Apache-2.0
 */

#include "service/bt_communication_service.hpp"
#include "esp_log.h"
// DT_INCLUDE

#ifdef __cplusplus
extern "C"
{
#endif

#include "nvs.h"
#include "nvs_flash.h"
#include "esp_bt_device.h"

#ifdef __cplusplus
}
#endif

#define CONFIG_EXAMPLE_SSP_ENABLED (true)
#define LOCAL_DEVICE_NAME ("MINTIA-BLUETOOTH-SPP")
#define SPP_SERVER_NAME "SPP_SERVER"

namespace service
{
    namespace
    {
        const char *kTag = "BtCommunicationService";
    }

    // シングルトンインスタンス用ポインタ
    BtCommunicationService *BtCommunicationService::instance_ = nullptr;

    /**
     * @brief コンストラクタ
     * @param handler 通信タスクハンドラへのポインタ
     */
    BtCommunicationService::BtCommunicationService(Task::ICommunicationTaskHandler *handler)
        : handler_(handler), sppConnected_(false)
    {
    }

    /**
     * @brief デストラクタ
     */
    BtCommunicationService::~BtCommunicationService()
    {
    }

    /**
     * @brief SPP通信サービスの初期化
     * @return true 初期化成功
     * @return false 初期化失敗
     */
    bool BtCommunicationService::Initialize()
    {
        char bda_str[18] = {0};
        ESP_LOGI(kTag, "SPP初期化処理 開始");

        // シングルトンインスタンス登録
        instance_ = this;

        esp_err_t ret = nvs_flash_init();
        if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND)
        {
            ESP_ERROR_CHECK(nvs_flash_erase());
            ret = nvs_flash_init();
        }
        ESP_ERROR_CHECK(ret);

        ESP_ERROR_CHECK(esp_bt_controller_mem_release(ESP_BT_MODE_BLE));

        esp_bt_controller_config_t btCfg = BT_CONTROLLER_INIT_CONFIG_DEFAULT();
        if ((ret = esp_bt_controller_init(&btCfg)) != ESP_OK)
        {
            ESP_LOGE(kTag, "%s initialize controller failed: %s", __func__, esp_err_to_name(ret));
            return false;
        }

        if ((ret = esp_bt_controller_enable(ESP_BT_MODE_CLASSIC_BT)) != ESP_OK)
        {
            ESP_LOGE(kTag, "%s enable controller failed: %s", __func__, esp_err_to_name(ret));
            return false;
        }

        esp_bluedroid_config_t bluedroidCfg = BT_BLUEDROID_INIT_CONFIG_DEFAULT();
#if (CONFIG_EXAMPLE_SSP_ENABLED == false)
        bluedroidCfg.ssp_en = false;
#endif
        if ((ret = esp_bluedroid_init_with_cfg(&bluedroidCfg)) != ESP_OK)
        {
            ESP_LOGE(kTag, "%s initialize bluedroid failed: %s", __func__, esp_err_to_name(ret));
            return false;
        }

        if ((ret = esp_bluedroid_enable()) != ESP_OK)
        {
            ESP_LOGE(kTag, "%s enable bluedroid failed: %s", __func__, esp_err_to_name(ret));
            return false;
        }

        if ((ret = esp_bt_gap_register_callback(GapCallback)) != ESP_OK)
        {
            ESP_LOGE(kTag, "%s gap register failed: %s", __func__, esp_err_to_name(ret));
            return false;
        }

        if ((ret = esp_spp_register_callback(SppCallback)) != ESP_OK)
        {
            ESP_LOGE(kTag, "%s spp register failed: %s", __func__, esp_err_to_name(ret));
            return false;
        }

        esp_spp_cfg_t btSppCfg = {
            .mode = ESP_SPP_MODE_CB,
            .enable_l2cap_ertm = true,
            .tx_buffer_size = 0, /* Only used for ESP_SPP_MODE_VFS mode */
        };
        if ((ret = esp_spp_enhanced_init(&btSppCfg)) != ESP_OK)
        {
            ESP_LOGE(kTag, "%s spp init failed: %s", __func__, esp_err_to_name(ret));
            return false;
        }

#if (CONFIG_EXAMPLE_SSP_ENABLED == true)
        /* Set default parameters for Secure Simple Pairing */
        esp_bt_sp_param_t param_type = ESP_BT_SP_IOCAP_MODE;
        esp_bt_io_cap_t iocap = ESP_BT_IO_CAP_IO;
        esp_bt_gap_set_security_param(param_type, &iocap, sizeof(uint8_t));
#endif

        /*
         * Set default parameters for Legacy Pairing
         * Use variable pin, input pin code when pairing
         */
        esp_bt_pin_type_t pin_type = ESP_BT_PIN_TYPE_VARIABLE;
        esp_bt_pin_code_t pin_code;
        esp_bt_gap_set_pin(pin_type, 0, pin_code);

        ESP_LOGI(kTag, "Own address:[%s]", BluetoothDevAddr2str((uint8_t *)esp_bt_dev_get_address(), bda_str, sizeof(bda_str)));
        return true;
    }

    /**
     * @brief SPP送受信処理の実行
     */
    void BtCommunicationService::Run()
    {
        ProcessSendData();
        // Bluetooth SPP受信処理はコールバックで処理される
    }

    // GAPコールバック関数（雛形）
    /**
     * @brief GAPコールバック関数
     * @param event GAPイベント種別
     * @param param GAPイベントパラメータ
     */
    void BtCommunicationService::GapCallback(esp_bt_gap_cb_event_t event, esp_bt_gap_cb_param_t *param)
    {
        char bda_str[18] = {0};
        ESP_LOGI(kTag, "GAP event: %d", event);

        switch (event)
        {
        case ESP_BT_GAP_AUTH_CMPL_EVT:
        {
            if (param->auth_cmpl.stat == ESP_BT_STATUS_SUCCESS)
            {
                ESP_LOGI(kTag, "authentication success: %s bda:[%s]", param->auth_cmpl.device_name,
                         BluetoothDevAddr2str(param->auth_cmpl.bda, bda_str, sizeof(bda_str)));
            }
            else
            {
                ESP_LOGE(kTag, "authentication failed, status:%d", param->auth_cmpl.stat);
            }
            break;
        }
        case ESP_BT_GAP_PIN_REQ_EVT:
        {
            ESP_LOGI(kTag, "ESP_BT_GAP_PIN_REQ_EVT min_16_digit:%d", param->pin_req.min_16_digit);
            if (param->pin_req.min_16_digit)
            {
                ESP_LOGI(kTag, "Input pin code: 0000 0000 0000 0000");
                esp_bt_pin_code_t pin_code = {0};
                esp_bt_gap_pin_reply(param->pin_req.bda, true, 16, pin_code);
            }
            else
            {
                ESP_LOGI(kTag, "Input pin code: 1234");
                esp_bt_pin_code_t pin_code;
                pin_code[0] = '1';
                pin_code[1] = '2';
                pin_code[2] = '3';
                pin_code[3] = '4';
                esp_bt_gap_pin_reply(param->pin_req.bda, true, 4, pin_code);
            }
            break;
        }

#if (CONFIG_EXAMPLE_SSP_ENABLED == true)
        case ESP_BT_GAP_CFM_REQ_EVT:
            ESP_LOGI(kTag, "ESP_BT_GAP_CFM_REQ_EVT Please compare the numeric value: %06" PRIu32, param->cfm_req.num_val);
            esp_bt_gap_ssp_confirm_reply(param->cfm_req.bda, true);
            break;
        case ESP_BT_GAP_KEY_NOTIF_EVT:
            ESP_LOGI(kTag, "ESP_BT_GAP_KEY_NOTIF_EVT passkey:%06" PRIu32, param->key_notif.passkey);
            break;
        case ESP_BT_GAP_KEY_REQ_EVT:
            ESP_LOGI(kTag, "ESP_BT_GAP_KEY_REQ_EVT Please enter passkey!");
            break;
#endif

        case ESP_BT_GAP_MODE_CHG_EVT:
            ESP_LOGI(kTag, "ESP_BT_GAP_MODE_CHG_EVT mode:%d bda:[%s]", param->mode_chg.mode,
                     BluetoothDevAddr2str(param->mode_chg.bda, bda_str, sizeof(bda_str)));
            break;

        default:
        {
            ESP_LOGI(kTag, "event: %d", event);
            break;
        }
        }
        return;
    }

    // SPPコールバック関数（静的メンバ関数として定義）
    /**
     * @brief SPPコールバック関数（静的メンバ関数）
     * @param event SPPイベント種別
     * @param param SPPイベントパラメータ
     */
    void BtCommunicationService::SppCallback(esp_spp_cb_event_t event, esp_spp_cb_param_t *param)
    {
        char bda_str[18] = {0};

        switch (event)
        {
        case ESP_SPP_INIT_EVT:
            if (param->init.status == ESP_SPP_SUCCESS)
            {
                ESP_LOGI(kTag, "ESP_SPP_INIT_EVT");
                esp_spp_start_srv(ESP_SPP_SEC_AUTHENTICATE, ESP_SPP_ROLE_SLAVE, 0, SPP_SERVER_NAME);
            }
            else
            {
                ESP_LOGE(kTag, "ESP_SPP_INIT_EVT status:%d", param->init.status);
            }
            break;
        case ESP_SPP_DISCOVERY_COMP_EVT:
            ESP_LOGI(kTag, "ESP_SPP_DISCOVERY_COMP_EVT");
            break;
        case ESP_SPP_OPEN_EVT:
            ESP_LOGI(kTag, "ESP_SPP_OPEN_EVT");
            // SPP接続確立時
            if (instance_ != nullptr)
            {
                instance_->sppConnected_ = true;
                instance_->sppHandle_ = param->open.handle;
                ESP_LOGI(kTag, "SPP接続状態: true, handle: %" PRIu32, param->open.handle);
            }
            break;
        case ESP_SPP_CLOSE_EVT:
            ESP_LOGI(kTag, "ESP_SPP_CLOSE_EVT status:%d handle:%" PRIu32 " close_by_remote:%d", param->close.status,
                     param->close.handle, param->close.async);
            // SPP切断時
            if (instance_ != nullptr)
            {
                instance_->sppConnected_ = false;
                instance_->sppHandle_ = 0;
                ESP_LOGI(kTag, "SPP接続状態: false, handle cleared");
            }
            break;
        case ESP_SPP_START_EVT:
            if (param->start.status == ESP_SPP_SUCCESS)
            {
                ESP_LOGI(kTag, "ESP_SPP_START_EVT handle:%" PRIu32 " sec_id:%d scn:%d", param->start.handle, param->start.sec_id,
                         param->start.scn);
                esp_bt_gap_set_device_name(LOCAL_DEVICE_NAME);
                esp_bt_gap_set_scan_mode(ESP_BT_CONNECTABLE, ESP_BT_GENERAL_DISCOVERABLE);
            }
            else
            {
                ESP_LOGE(kTag, "ESP_SPP_START_EVT status:%d", param->start.status);
            }
            break;
        case ESP_SPP_CL_INIT_EVT:
            ESP_LOGI(kTag, "ESP_SPP_CL_INIT_EVT");
            break;
        case ESP_SPP_DATA_IND_EVT:
            ESP_LOGI(kTag, "ESP_SPP_DATA_IND_EVT len:%d handle:%" PRIu32,
                     param->data_ind.len, param->data_ind.handle);
            if (instance_ != nullptr && instance_->handler_)
            {
                instance_->handler_->OnReceive(param->data_ind.data, param->data_ind.len);
            }
            break;
        case ESP_SPP_CONG_EVT:
            ESP_LOGI(kTag, "ESP_SPP_CONG_EVT");
            break;
        case ESP_SPP_WRITE_EVT:
            ESP_LOGI(kTag, "ESP_SPP_WRITE_EVT");
            break;
        case ESP_SPP_SRV_OPEN_EVT:
            ESP_LOGI(kTag, "ESP_SPP_SRV_OPEN_EVT status:%d handle:%" PRIu32 ", rem_bda:[%s]", param->srv_open.status,
                     param->srv_open.handle, BluetoothDevAddr2str(param->srv_open.rem_bda, bda_str, sizeof(bda_str)));
            // SPP接続確立時
            if (instance_ != nullptr)
            {
                instance_->sppConnected_ = true;
                instance_->sppHandle_ = param->srv_open.handle;
                ESP_LOGI(kTag, "SPP接続状態: true, handle: %" PRIu32, param->srv_open.handle);
            }
            break;
        case ESP_SPP_SRV_STOP_EVT:
            ESP_LOGI(kTag, "ESP_SPP_SRV_STOP_EVT");
            break;
        case ESP_SPP_UNINIT_EVT:
            ESP_LOGI(kTag, "ESP_SPP_UNINIT_EVT");
            break;
        default:
            break;
        }
    }

    /**
     * @brief SPPでデータを送信する
     * @param data 送信データ
     * @param len データ長
     */
    void BtCommunicationService::ProcessSendData()
    {
        if (!sppConnected_ || !handler_)
        {
            return;
        }

        const uint8_t *txBuf = handler_->GetTxMessageCache();
        size_t txLen = handler_->GetTxMessageCacheLen();
        if (txLen == 0)
        {
            ESP_LOGD(kTag, "No data to send (ProcessSendData)");
            return;
        }

        // 送信データが文字列である保証がない場合は16進ダンプ等に変更も検討
        ESP_LOGI(kTag, "Sending %zu bytes (ProcessSendData)", txLen);
        esp_spp_write(sppHandle_, txLen, const_cast<uint8_t *>(txBuf));
    }

    /**
     * @brief Bluetoothデバイスアドレスを文字列に変換する
     *
     * @param bda Bluetoothデバイスアドレス
     * @param str 変換結果を格納するバッファ
     * @param size バッファサイズ
     * @return char*
     */
    char *BtCommunicationService::BluetoothDevAddr2str(uint8_t *bda, char *str, size_t size)
    {
        if (bda == NULL || str == NULL || size < 18)
        {
            return NULL;
        }

        uint8_t *p = bda;
        sprintf(str, "%02x:%02x:%02x:%02x:%02x:%02x",
                p[0], p[1], p[2], p[3], p[4], p[5]);
        return str;
    }

} // namespace service
