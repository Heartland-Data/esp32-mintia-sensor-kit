/**
 * @file data_parser_service.hpp
 * @brief PCコマンドインターフェースのデータ解析サービス定義
 *
 * @copyright Copyright (C) 2026, Heartland Data inc. All Rights Reserved.
 *
 * SPDX-FileCopyrightText: 2026 Heartland Data inc.
 * SPDX-License-Identifier: Apache-2.0
 */
#pragma once
#include <cstddef>
#include <cstdint>
#include <vector>
#include <memory>
#include "command_interface/communication_packet.hpp"
#include "idata_parser_task_handler.hpp"

namespace service
{
    using SendResponseCallback = bool (*)(const uint8_t *data, size_t len, void *userCtx);

    class IDataParserService
    {
    public:
        virtual ~IDataParserService() = default;
        virtual void Parse(const uint8_t *data, size_t len) = 0;
    };

    class DataParserService : public IDataParserService
    {
    public:
        // ハンドラーを受け取るコンストラクタ
        DataParserService(Task::IDataParserTaskHandler *handler);
        void Parse(const uint8_t *data, size_t len) override;

    private:
        Task::IDataParserTaskHandler *handler_;
        std::vector<uint8_t> rxBuffer_;
        // 抽出済みパケット格納用
        std::vector<std::unique_ptr<Domain::CommandInterface::CommunicationPacket>> parsedPackets_;

        void ExtractPackets();
        void AnalyzePackets();
        void AnalyzeCommandRequest(std::unique_ptr<Domain::CommandInterface::CommunicationPacket> pkt);
        void SendSuccessResponse(const std::string &message);
    };
} // namespace service
