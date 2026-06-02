/**
 * @file communication_packet.cpp
 * @brief 通信用パケットクラス実装
 * @copyright Copyright (C) 2026, Heartland Data inc. All Rights Reserved.
 * SPDX-FileCopyrightText: 2026 Heartland Data inc.
 * SPDX-License-Identifier: Apache-2.0
 */
#include "command_interface/communication_packet.hpp"

namespace Domain
{
    namespace CommandInterface
    {
        namespace
        {
            // uint16_t→リトルエンディアン2バイト
            /**
             * @brief uint16_t値をリトルエンディアン2バイトで出力ベクターに追加する
             * @param val 追加する値
             * @param out 出力先ベクター
             */
            void WriteLE16(uint16_t val, std::vector<uint8_t> &out)
            {
                out.push_back(static_cast<uint8_t>(val & 0xFF));
                out.push_back(static_cast<uint8_t>((val >> 8) & 0xFF));
            }
        }

        /**
         * @brief リトルエンディアン2バイトからuint16_t値を取得する
         * @param p 2バイトの配列先頭ポインタ
         * @return uint16_t値
         */
        uint16_t CommunicationPacket::ReadLE16(const uint8_t *p)
        {
            return static_cast<uint16_t>(p[0]) | (static_cast<uint16_t>(p[1]) << 8);
        }

        /**
         * @brief パケット生成（データから）
         * @param length データ長
         * @param type パケット種別
         * @param data データ本体
         * @return 生成されたCommunicationPacketのユニークポインタ（失敗時nullptr）
         */
        std::unique_ptr<CommunicationPacket> CommunicationPacket::Create(uint16_t length, uint8_t type, const std::vector<uint8_t> &data)
        {
            if (data.size() != length)
            {
                return nullptr;
            }
            return std::unique_ptr<CommunicationPacket>(new CommunicationPacket(0x5AA5, length, type, std::vector<uint8_t>(data)));
        }

        /**
         * @brief バイナリ列からパケット生成
         * @param binary バイナリデータ
         * @return 生成されたCommunicationPacketのユニークポインタ（失敗時nullptr）
         */
        std::unique_ptr<CommunicationPacket> CommunicationPacket::CreateFromBinary(const std::vector<uint8_t> &binary)
        {
            if (binary.size() < CommunicationPacket::kHeaderSize)
            {
                return nullptr;
            }

            uint16_t sync = ReadLE16(&binary[CommunicationPacket::kSyncOffset]);
            if (sync != 0x5AA5)
            {
                return nullptr;
            }

            uint16_t length = ReadLE16(&binary[CommunicationPacket::kLengthOffset]);
            uint8_t type = binary[CommunicationPacket::kTypeOffset];

            if (binary.size() < CommunicationPacket::kHeaderSize + length)
            {
                return nullptr;
            }

            std::vector<uint8_t> data;
            if (length > 0)
            {
                data.assign(binary.begin() + CommunicationPacket::kDataOffset, binary.begin() + CommunicationPacket::kDataOffset + length);
            }

            return std::unique_ptr<CommunicationPacket>(new CommunicationPacket(sync, length, type, std::move(data)));
        }

        /**
         * @brief パケットをバイナリ列に変換
         * @return バイナリデータ
         */
        std::vector<uint8_t> CommunicationPacket::ToBinary() const
        {
            std::vector<uint8_t> bin;
            WriteLE16(sync_, bin);
            WriteLE16(length_, bin);
            bin.push_back(type_);
            bin.insert(bin.end(), data_.begin(), data_.end());
            return bin;
        }

        /**
         * @brief コンストラクタ
         * @param sync シンクワード
         * @param length データ長
         * @param type パケット種別
         * @param data データ本体（ムーブ）
         */
        CommunicationPacket::CommunicationPacket(uint16_t sync, uint16_t length, uint8_t type, std::vector<uint8_t> &&data)
            : sync_(sync), length_(length), type_(type), data_(std::move(data))
        {
        }
    } // namespace CommandInterface
} // namespace Domain
