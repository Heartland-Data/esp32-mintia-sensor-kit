# MLX90640 ESP32-IDF Library

このライブラリは、SparkFun の MLX90640 IR アレイセンサー用の ESP32-IDF ライブラリです。Arduino 環境向けのコードから ESP32-IDF 向けに移植されています。

## 特徴

- MLX90640 32x24 IR アレイセンサーの完全サポート
- ESP32-IDF I2C ドライバとの統合
- 温度計算と RAW IR イメージデータの取得
- 設定可能なリフレッシュレートと解像度
- 使いやすいサービスクラス

## ファイル構成

```
components/mlx90640/
├── CMakeLists.txt
├── README.md
├── include/
│   ├── mlx90640_api.h          # MLX90640 API関数
│   ├── mlx90640_i2c_driver.h   # I2C通信ドライバ
│   └── mlx90640_service.h      # 高レベルサービスクラス
├── mlx90640_api.c              # MLX90640 API実装
├── mlx90640_i2c_driver.c       # I2C通信ドライバ実装
└── mlx90640_service.c          # 高レベルサービス実装
```

## 使用方法

### 1. 基本的な初期化と温度取得

```c
#include "mlx90640_service.h"

void app_main()
{
    // 設定の初期化
    mlx90640_config_t config = MLX90640_DEFAULT_CONFIG();

    // サービスの初期化
    mlx90640_service_t *mlx_service = mlx90640_service_init(&config);
    if (mlx_service == NULL) {
        ESP_LOGE("APP", "Failed to initialize MLX90640 service");
        return;
    }

    // 温度データ配列 (24x32 = 768個)
    float temperatures[768];

    while (1) {
        // 温度データの取得
        esp_err_t ret = mlx90640_service_get_temperatures(mlx_service, temperatures);
        if (ret == ESP_OK) {
            // 温度データの処理
            for (int i = 0; i < 768; i++) {
                printf("%.2f,", temperatures[i]);
                if ((i + 1) % 32 == 0) printf("\n");
            }
        }

        vTaskDelay(pdMS_TO_TICKS(500)); // 500ms待機
    }

    // クリーンアップ
    mlx90640_service_deinit(mlx_service);
}
```

### 2. 既存のタスクシステムとの統合

```c
#include "mlx90640_service.h"

// タスククラスのヘッダで
class YourIrSensorTask {
private:
    mlx90640_service_t *mlx_service_;

public:
    bool Initialize() {
        mlx90640_config_t config = MLX90640_DEFAULT_CONFIG();
        // カスタム設定
        config.sda_pin = 21;
        config.scl_pin = 22;
        config.refresh_rate = 0x03; // 4Hz

        mlx_service_ = mlx90640_service_init(&config);
        return (mlx_service_ != nullptr);
    }

    void Run() {
        float temperatures[768];
        esp_err_t ret = mlx90640_service_get_temperatures(mlx_service_, temperatures);
        if (ret == ESP_OK) {
            // データをメッセージバッファに送信
            SendSensorData((uint8_t*)temperatures, sizeof(temperatures));
        }
    }

    ~YourIrSensorTask() {
        if (mlx_service_) {
            mlx90640_service_deinit(mlx_service_);
        }
    }
};
```

### 3. 低レベル API 使用例

```c
#include "mlx90640_api.h"
#include "mlx90640_i2c_driver.h"

void low_level_example()
{
    // I2C初期化
    MLX90640_I2CInit(I2C_NUM_0, 21, 22, 400000);

    uint8_t mlx_addr = 0x33;
    uint16_t eeMLX90640[832];
    paramsMLX90640 mlx90640;

    // EEPROM読み込み
    int status = MLX90640_DumpEE(mlx_addr, eeMLX90640);
    if (status != 0) {
        ESP_LOGE("MLX", "Failed to load system parameters");
        return;
    }

    // パラメータ抽出
    status = MLX90640_ExtractParameters(eeMLX90640, &mlx90640);
    if (status != 0) {
        ESP_LOGE("MLX", "Parameter extraction failed");
        return;
    }

    // リフレッシュレート設定
    MLX90640_SetRefreshRate(mlx_addr, 0x03); // 4Hz

    uint16_t mlx90640Frame[834];
    float mlx90640To[768];

    while (1) {
        // フレームデータ取得
        status = MLX90640_GetFrameData(mlx_addr, mlx90640Frame);
        if (status < 0) {
            ESP_LOGE("MLX", "GetFrame Error: %d", status);
            continue;
        }

        float vdd = MLX90640_GetVdd(mlx90640Frame, &mlx90640);
        float ta = MLX90640_GetTa(mlx90640Frame, &mlx90640);
        float tr = ta - 8; // 反射温度
        float emissivity = 0.95;

        // 温度計算
        MLX90640_CalculateTo(mlx90640Frame, &mlx90640, emissivity, tr, mlx90640To);

        // データ処理
        for (int x = 0; x < 768; x++) {
            printf("%.2f,", mlx90640To[x]);
        }
        printf("\n");

        vTaskDelay(pdMS_TO_TICKS(250));
    }
}
```

## 設定パラメータ

### リフレッシュレート

- 0x00: 0.5Hz
- 0x01: 1Hz
- 0x02: 2Hz
- 0x03: 4Hz (推奨)
- 0x04: 8Hz
- 0x05: 16Hz
- 0x06: 32Hz
- 0x07: 64Hz

### 解像度

- 0x00: 16 ビット
- 0x01: 17 ビット (推奨)
- 0x02: 18 ビット
- 0x03: 19 ビット

## ハードウェア接続

```
ESP32    MLX90640
-----    --------
3.3V  -> VCC
GND   -> GND
21    -> SDA
22    -> SCL
```

プルアップ抵抗（4.7kΩ）が SDA と SCL ラインに必要です。

## トラブルシューティング

### I2C 通信エラー

- 配線を確認してください
- プルアップ抵抗が正しく接続されているか確認してください
- I2C アドレス（デフォルト：0x33）を確認してください

### データ取得エラー

- センサーの電源投入後、100ms 以上待機してください
- フレームレート設定を下げてみてください

## ライセンス

このライブラリは **Apache License 2.0** の下でライセンスされています（`LICENSE` ファイル参照）。

### 著作権表示・帰属情報

本コードは以下の著作物を元に派生したものです。

| 著作物 | 著作権者 | ライセンス | URL |
|---|---|---|---|
| MLX90640 library (オリジナル) | © 2017 Melexis N.V. | Apache License 2.0 | https://github.com/melexis/mlx90640-library |
| Qwiic IR Array MLX90640 Firmware (Arduino 移植) | SparkFun Electronics | Apache License 2.0 ※ | https://github.com/sparkfun/Qwiic_IR_Array_MLX90640 |
| 本ライブラリ (ESP32-IDF 移植・改変) | © 2025 Heartland.Data Inc. | Apache License 2.0 | — |

※ SparkFun のファームウェアは Melexis の Apache License 2.0 ヘッダをそのまま維持しています。

Apache License 2.0 の全文は [http://www.apache.org/licenses/LICENSE-2.0](http://www.apache.org/licenses/LICENSE-2.0) を参照してください。
