#pragma once

/**
 * @file mlx90640_ir_sensor_service.hpp
 * @brief MLX90640用IRセンササービス宣言
 * @copyright Copyright (C) 2026, Heartland Data inc. All Rights Reserved.
 * SPDX-FileCopyrightText: 2026 Heartland Data inc.
 * SPDX-License-Identifier: Apache-2.0
 */

#include "isensor_service.hpp"
#include "mlx90640_api.h"
#include "mlx90640_i2c_driver.h"
#include "isensor_task_handler.hpp"

namespace service
{
    // MLX90640データ仕様に基づく定数
    namespace MLX90640Spec
    {
        static constexpr int kPixelRows = 24;                            // IRアレイの行数
        static constexpr int kPixelCols = 32;                            // IRアレイの列数
        static constexpr int kPixelCount = kPixelRows * kPixelCols;      // 768ピクセル
        static constexpr int kFrameAuxWords = 66;                        // 補助データワード数（RAM 0x0700～0x073F + 追加2ワード）
        static constexpr int kFrameWords = kPixelCount + kFrameAuxWords; // 834ワード（768 + 66）
        static constexpr int kEepromWords = 832;                         // EEPROM全体のワード数
    }

    /**
     * @brief MLX90640用IRセンササービス
     */
    class Mlx90640IrSensorService : public ISensorService
    {
    public:
        Mlx90640IrSensorService(Task::ISensorTaskHandler *handler);
        ~Mlx90640IrSensorService() override;

        bool Initialize() override;
        void Run() override;

    private:
        Task::ISensorTaskHandler *handler_;
        bool InitializeMlx90640();

        void SendMlx90640SensorData(const float *pixels, size_t pixelCount, float ambientTemp);

        // MLX90640 sensor data
        paramsMLX90640 params_;

        // frameData_配列の内訳（総834ワード）：
        //  - 768ワード: 32×24ピクセルのIRデータ（RAMアドレス 0x0400～0x06FF）
        //  - 66ワード: 補助データ（RAMアドレス 0x0700～0x073F + ステータス情報等）
        //    * Ta_Vbe, Ta_PTAT（周囲温度センサデータ）
        //    * CP(SP0), CP(SP1)（補償ピクセルデータ）
        //    * GAIN, VDDpix（ゲイン・電源電圧データ）
        //    * その他制御・ステータス情報
        uint16_t frameData_[MLX90640Spec::kFrameWords];

        // eeData_配列（832ワード）：
        //  - EEPROM全体のキャリブレーションデータ
        //  - センサ個体ごとの補正パラメータを格納
        uint16_t eeData_[MLX90640Spec::kEepromWords];

        // Configuration
        uint8_t deviceAddress_;
        i2c_port_t i2cPort_;
        int sdaPin_;
        int sclPin_;
        uint32_t i2cFreqHz_;
        uint8_t refreshRate_;
        uint8_t resolution_;

        bool sensorInitialized_;
    };

} // namespace service
