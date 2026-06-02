/**
 * @file data_parser_service.cpp
 * @brief PCコマンドインターフェースのデータ解析サービス実装
 *
 * @copyright Copyright (C) 2026, Heartland Data inc. All Rights Reserved.
 *
 * SPDX-FileCopyrightText: 2026 Heartland Data inc.
 * SPDX-License-Identifier: Apache-2.0
 */
#include "data_parser_service.hpp"
#include "esp_log.h"
#include <algorithm>
#include <iterator>
#include "command_interface/communication_packet.hpp"
#include "command_interface/command_request.hpp"
#include "command_interface/command_response.hpp"

namespace service
{
    namespace
    {
        const char *kTag = "DataParserService";
    }

    /**
     * @brief DataParserService のコンストラクタ
     */
    DataParserService::DataParserService(Task::IDataParserTaskHandler *handler)
        : handler_(handler)
    {
    }

    /**
     * @brief データ解析サービスのパース処理
     *
     * @param data 受信データ
     * @param len データ長
     */
    void DataParserService::Parse(const uint8_t *data, size_t len)
    {
        // 受信データをバッファに追記
        rxBuffer_.insert(rxBuffer_.end(), data, data + len);
        ExtractPackets();
        AnalyzePackets();
    }

    /**
     * @brief データ解析サービスのパケット抽出処理
     *
     */
    void DataParserService::ExtractPackets()
    {
        using Domain::CommandInterface::CommunicationPacket;
        while (true)
        {
            // SYNCパターン検索
            auto it = rxBuffer_.begin();
            while (std::distance(it, rxBuffer_.end()) >= CommunicationPacket::kSyncSize)
            {
                uint16_t sync = CommunicationPacket::ReadLE16(&(*it));
                if (sync == CommunicationPacket::kSyncPattern)
                {
                    ESP_LOGI(kTag, "SYNC found at position %zu", std::distance(rxBuffer_.begin(), it));
                    break;
                }
                ++it;
            }
            if (std::distance(it, rxBuffer_.end()) < CommunicationPacket::kSyncSize)
            {
                // SYNCが見つからない場合はバッファをクリア
                rxBuffer_.clear();
                ESP_LOGW(kTag, "SYNC not found, clearing buffer");
                break;
            }
            // SYNCまで不要データを削除
            rxBuffer_.erase(rxBuffer_.begin(), it);

            if (rxBuffer_.size() < CommunicationPacket::kHeaderSize)
            {
                ESP_LOGW(kTag, "Buffer too small for header, waiting for more data");
                break; // ヘッダ不足
            }

            // LENGTH取得（Little Endian）
            uint16_t length = CommunicationPacket::ReadLE16(&rxBuffer_[CommunicationPacket::kLengthOffset]);
            size_t packetSize = CommunicationPacket::kHeaderSize + length;
            if (rxBuffer_.size() < packetSize)
            {
                ESP_LOGW(kTag, "Buffer too small for packet (expected %zu, got %zu)", packetSize, rxBuffer_.size());
                break; // パケット全体が揃っていない
            }

            // パケット切り出し
            std::vector<uint8_t> packet(rxBuffer_.begin(), rxBuffer_.begin() + packetSize);

            // パケットオブジェクト生成＆ログ出力
            auto commPacket = CommunicationPacket::CreateFromBinary(packet);
            if (commPacket)
            {
                const auto &data = commPacket->GetData();
                ESP_LOGI(kTag, "Packet parsed: type=%d, length=%d, data=%.*s",
                         commPacket->GetType(),
                         commPacket->GetLength(),
                         static_cast<int>(data.size()),
                         reinterpret_cast<const char *>(data.data()));
                // パケットを格納
                parsedPackets_.push_back(std::move(commPacket));
            }
            else
            {
                ESP_LOGW(kTag, "Packet parse error: CreateFromBinary returned nullptr");
            }

            // バッファからパケット分を削除
            rxBuffer_.erase(rxBuffer_.begin(), rxBuffer_.begin() + packetSize);
        }
    }

    /**
     * @brief パケット解析処理
     *
     */
    void DataParserService::AnalyzePackets()
    {
        if (parsedPackets_.empty())
        {
            ESP_LOGI(kTag, "No packets to analyze.");
            return;
        }

        // 仮：パケットごとにタイプで分岐（今はログのみ）
        for (auto &pkt : parsedPackets_)
        {
            switch (pkt->GetPacketType())
            {
            case Domain::CommandInterface::PacketType::CommandRequest:
                ESP_LOGI(kTag, "Analyze: CommandRequest (PC→ESP32)");
                AnalyzeCommandRequest(std::move(pkt));
                break;
            case Domain::CommandInterface::PacketType::CommandResponse:
                ESP_LOGE(kTag, "Analyze: CommandResponse (ESP32→PC) - unexpected packet received");
                break;
            case Domain::CommandInterface::PacketType::SensorData:
                ESP_LOGE(kTag, "Analyze: SensorData (ESP32→PC) - unexpected packet received");
                break;
            default:
                ESP_LOGI(kTag, "Analyze: Unknown type=%d", pkt->GetType());
                break;
            }
        }
        // 解析後はクリア
        parsedPackets_.clear();
    }

    /**
     * @brief CommandRequestパケットの解析
     *
     * @param pkt
     */
    void DataParserService::AnalyzeCommandRequest(std::unique_ptr<Domain::CommandInterface::CommunicationPacket> pkt)
    {
        const auto &data = pkt->GetData();
        std::string jsonStr(data.begin(), data.end());
        auto cmdReq = Domain::CommandInterface::CommandRequest::CreateFromJson(jsonStr);
        if (cmdReq)
        {
            ESP_LOGI(kTag, "CommandRequest created: cmd=%s", cmdReq->GetCmd().c_str());
            const std::string &cmd = cmdReq->GetCmd();
            if (cmd == Domain::CommandInterface::kCmdSensorStart)
            {
                ESP_LOGI(kTag, "処理: sensor_startコマンド");
                // sensor_startコマンド用処理
                // TODO: 現状は無条件で成功を送付する
                SendSuccessResponse("Sensor started successfully");
                handler_->SendSensorCommand(
                    Domain::CommandInterface::SensorCommandMsg(Domain::CommandInterface::SensorCommandType::kStart));
            }
            else if (cmd == Domain::CommandInterface::kCmdSensorStop)
            {
                ESP_LOGI(kTag, "処理: sensor_stopコマンド");
                // sensor_stopコマンド用処理
                // TODO: 現状は無条件で成功を送付する
                SendSuccessResponse("Sensor stopped successfully");
                handler_->SendSensorCommand(
                    Domain::CommandInterface::SensorCommandMsg(Domain::CommandInterface::SensorCommandType::kStop));
            }
            else if (cmd == Domain::CommandInterface::kCmdGetInfo)
            {
                ESP_LOGI(kTag, "処理: get_infoコマンド");
                // get_infoコマンド用処理
            }
            else
            {
                ESP_LOGI(kTag, "Unknown command: %s", cmd.c_str());
                // その他コマンド
            }
        }
        else
        {
            ESP_LOGW(kTag, "Failed to create CommandRequest from JSON");
        }
    }

    /**
     * @brief 成功レスポンスを送信する
     *
     * @param message レスポンスメッセージ
     */
    void DataParserService::SendSuccessResponse(const std::string &message)
    {
        auto response = Domain::CommandInterface::CommandResponse::Create(
            Domain::CommandInterface::kCmdSensorStart, Domain::CommandInterface::kResultOk, message);
        if (response && handler_)
        {
            const std::string &json = response->ToJsonString();
            auto packet = Domain::CommandInterface::CommunicationPacket::Create(
                json.size(),
                static_cast<uint8_t>(Domain::CommandInterface::PacketType::CommandResponse),
                std::vector<uint8_t>(json.begin(), json.end()));
            handler_->SendResponse(packet->ToBinary().data(), packet->ToBinary().size());
        }
    }
} // namespace service
