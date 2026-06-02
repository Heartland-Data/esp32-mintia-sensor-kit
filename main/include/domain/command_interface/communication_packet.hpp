/**
 * @file communication_packet.hpp
 * @brief 通信用パケットクラス定義
 * @copyright Copyright (C) 2026, Heartland Data inc. All Rights Reserved.
 * SPDX-FileCopyrightText: 2026 Heartland Data inc.
 * SPDX-License-Identifier: Apache-2.0
 */
#pragma once

#include <cstdint>
#include <vector>
#include <memory>
#include <stdexcept>

namespace Domain
{
    namespace CommandInterface
    {
        enum class PacketType : uint8_t
        {
            CommandRequest = 0x10,
            CommandResponse = 0x20,
            SensorData = 0x30
        };

        class CommunicationPacket
        {
        public:
            static constexpr size_t kSyncSize = 2;
            static constexpr size_t kLengthSize = 2;
            static constexpr size_t kTypeSize = 1;
            static constexpr uint16_t kSyncPattern = 0x5AA5;
            static constexpr size_t kSyncOffset = 0;
            static constexpr size_t kLengthOffset = kSyncOffset + kSyncSize;
            static constexpr size_t kTypeOffset = kLengthOffset + kLengthSize;
            static constexpr size_t kDataOffset = kTypeOffset + kTypeSize;
            static constexpr size_t kHeaderSize = kSyncSize + kLengthSize + kTypeSize; // SYNC + LENGTH + TYPE

            // バイナリ列から生成
            static std::unique_ptr<CommunicationPacket> CreateFromBinary(const std::vector<uint8_t>& binary);

            // リトルエンディアン2バイト→uint16_t
            static uint16_t ReadLE16(const uint8_t *p);

            // length, type, dataから生成（SYNCは0x5AA5固定）
            static std::unique_ptr<CommunicationPacket> Create(uint16_t length, uint8_t type, const std::vector<uint8_t>& data);

            // メンバ変数からバイナリ列へ変換
            std::vector<uint8_t> ToBinary() const;

            // アクセサ
            uint16_t GetSync() const { return sync_; }
            uint16_t GetLength() const { return length_; }
            uint8_t GetType() const { return type_; }
            PacketType GetPacketType() const { return static_cast<PacketType>(type_); }
            const std::vector<uint8_t>& GetData() const { return data_; }

        private:
            // コンストラクタはprivate
            CommunicationPacket(uint16_t sync, uint16_t length, uint8_t type, std::vector<uint8_t>&& data);

            uint16_t sync_;
            uint16_t length_;
            uint8_t type_;
            std::vector<uint8_t> data_;
        };
    } // namespace CommandInterface
} // namespace Domain
