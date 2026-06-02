# AMG88xx ESP32 Driver

ESP-IDF対応のAMG88xx赤外線温度センサードライバーです。

## 概要

このライブラリは、Panasonic AMG88xx (AMG8833) 赤外線アレイセンサーをESP32で使用するためのESP-IDF互換ドライバーです。Arduino環境の依存関係を取り除き、ESP-IDFのAPIを使用して実装されています。

## 特徴

- ESP-IDF v4.4以降対応
- C言語インターフェース
- 8x8ピクセルの温度データ読み取り
- サーミスタ温度読み取り
- 割り込み機能サポート
- 移動平均フィルタ
- パワーマネジメント
- フレームレート設定（1FPS/10FPS）

## ハードウェア要件

- ESP32系マイコン
- AMG8833赤外線アレイセンサー
- I2C接続（SCL, SDA）

## 配線

| AMG8833 | ESP32 |
|---------|-------|
| VDD     | 3.3V  |
| GND     | GND   |
| SCL     | GPIO22 (設定可能) |
| SDA     | GPIO21 (設定可能) |
| INT     | GPIO (オプション) |

## 使用方法

### 1. I2Cの初期化

```c
#include "driver/i2c.h"

static esp_err_t i2c_master_init(void)
{
    i2c_config_t conf = {
        .mode = I2C_MODE_MASTER,
        .sda_io_num = 21,
        .scl_io_num = 22,
        .sda_pullup_en = GPIO_PULLUP_ENABLE,
        .scl_pullup_en = GPIO_PULLUP_ENABLE,
        .master.clk_speed = 400000,
    };

    esp_err_t err = i2c_param_config(I2C_NUM_0, &conf);
    if (err != ESP_OK) {
        return err;
    }

    return i2c_driver_install(I2C_NUM_0, conf.mode, 0, 0, 0);
}
```

### 2. AMG88xxの初期化

```c
#include "amg88xx_esp32.h"

amg88xx_handle_t amg_handle;
amg88xx_config_t amg_config = {
    .i2c_port = I2C_NUM_0,
    .device_address = AMG88XX_ADDRESS,
    .timeout_ms = 1000
};

esp_err_t ret = amg88xx_init(&amg_handle, &amg_config);
if (ret != ESP_OK) {
    ESP_LOGE(TAG, "Failed to initialize AMG88xx");
    return;
}
```

### 3. 温度データの読み取り

```c
// ピクセル温度の読み取り
float pixels[AMG88XX_PIXEL_ARRAY_SIZE];
ret = amg88xx_read_pixels(&amg_handle, pixels, AMG88XX_PIXEL_ARRAY_SIZE);
if (ret == ESP_OK) {
    // 8x8の温度データを処理
    for (int i = 0; i < 64; i++) {
        printf("Pixel %d: %.2f°C\n", i, pixels[i]);
    }
}

// サーミスタ温度の読み取り
float thermistor_temp;
ret = amg88xx_read_thermistor(&amg_handle, &thermistor_temp);
if (ret == ESP_OK) {
    printf("Thermistor: %.2f°C\n", thermistor_temp);
}
```

### 4. 設定変更

```c
// フレームレート設定
amg88xx_set_frame_rate(&amg_handle, AMG88XX_FPS_10);

// 移動平均フィルタ
amg88xx_set_moving_average_mode(&amg_handle, true);

// パワーモード設定
amg88xx_set_power_mode(&amg_handle, AMG88XX_NORMAL_MODE);
```

### 5. 割り込み設定

```c
// 割り込みレベル設定
amg88xx_set_interrupt_levels(&amg_handle, 30.0f, 25.0f);

// 割り込み有効化
amg88xx_enable_interrupt(&amg_handle);

// 割り込みモード設定
amg88xx_set_interrupt_mode(&amg_handle, AMG88XX_ABSOLUTE_VALUE);
```

## API リファレンス

### 初期化・終了処理

- `amg88xx_init()` - センサーの初期化
- `amg88xx_deinit()` - センサーの終了処理

### データ読み取り

- `amg88xx_read_pixels()` - ピクセル温度データ読み取り（float値）
- `amg88xx_read_pixels_raw()` - ピクセル温度データ読み取り（生データ）
- `amg88xx_read_thermistor()` - サーミスタ温度読み取り

### 設定

- `amg88xx_set_power_mode()` - パワーモード設定
- `amg88xx_set_frame_rate()` - フレームレート設定
- `amg88xx_set_moving_average_mode()` - 移動平均フィルタ設定
- `amg88xx_software_reset()` - ソフトウェアリセット

### 割り込み

- `amg88xx_enable_interrupt()` - 割り込み有効化
- `amg88xx_disable_interrupt()` - 割り込み無効化
- `amg88xx_set_interrupt_mode()` - 割り込みモード設定
- `amg88xx_set_interrupt_levels()` - 割り込みレベル設定
- `amg88xx_get_interrupt()` - 割り込み状態読み取り
- `amg88xx_clear_interrupt()` - 割り込みクリア

## サンプルコード

詳細な使用例は `amg88xx_example.c` を参照してください。

## ライセンス

このドライバは Adafruit 社の AMG88xx Arduino ライブラリ
([https://github.com/adafruit/Adafruit_AMG88xx](https://github.com/adafruit/Adafruit_AMG88xx))
を基に ESP-IDF 向けに移植したものです。

**元ライブラリの著作権表示：**

```
MIT License

Copyright (c) Adafruit Industries
Written by Dean Miller for Adafruit Industries
```

本ファイルを含む派生著作物には MIT ライセンスが適用されます。
ライセンス全文は `LICENSE` ファイルを参照してください。

## 注意事項

1. I2Cバスは事前に初期化されている必要があります
2. センサーの電源は3.3Vを使用してください
3. プルアップ抵抗がI2C信号線に必要です
4. センサーの視野角は60度です
5. 動作温度範囲：-20°C ～ +80°C
6. 測定温度範囲：0°C ～ +80°C
