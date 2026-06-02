/**
 * @file uart_log.c
 * @brief UART2を用いたログ出力機能の実装ファイル
 * @copyright Copyright (C) 2026, Heartland Data inc. All Rights Reserved.
 * SPDX-FileCopyrightText: 2026 Heartland Data inc.
 * SPDX-License-Identifier: Apache-2.0
 */

#include "uart_log.h"
#include "driver/uart.h"
#include "driver/gpio.h"
#include "esp_log.h"
#include <stdio.h>
#include <string.h>

#define UART2_LOG_PRINT_BUFFER_SIZE (512)
#define UART2_LOG_UART_RX_BUFFER_SIZE (1024)
#define UART2_LOG_RX_FLOW_CTRL_THRESH (122) // FIFO(128B)の残り6BでRTSをアサートする推奨値(ESP-IDFのドキュメント参照)

/**
 * @brief UART2用のprintfハンドラ
 * @param format 書式文字列
 * @param args 可変引数リスト
 * @return 出力した文字数
 * @details ESP-IDFのesp_log_set_vprintf用
 */
int uart2_printf(const char *format, va_list args)
{
    char print_buffer[UART2_LOG_PRINT_BUFFER_SIZE];
    int len = vsnprintf(print_buffer, sizeof(print_buffer), format, args);
    if (len > 0)
    {
        if (len > UART2_LOG_PRINT_BUFFER_SIZE - 1)
        {
            len = UART2_LOG_PRINT_BUFFER_SIZE - 1;
        }
        uart_write_bytes(UART2_LOG_UART_NUM, print_buffer, len);
    }
    return len;
}

/**
 * @brief UART2の初期化とログ出力先設定
 * @details UART2を初期化し、ログ出力先をUART2に設定する
 */
void init_uart2_for_log(void)
{
    const uart_config_t uart_config = {
        .baud_rate = UART2_BAUD_RATE,
        .data_bits = UART_DATA_8_BITS,
        .parity = UART_PARITY_DISABLE,
        .stop_bits = UART_STOP_BITS_1,
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
        .rx_flow_ctrl_thresh = UART2_LOG_RX_FLOW_CTRL_THRESH,
        .source_clk = UART_SCLK_DEFAULT,
        .flags = {
            .allow_pd = 0,
            .backup_before_sleep = 0,
        },
    };

    uart_param_config(UART2_LOG_UART_NUM, &uart_config);
    uart_set_pin(UART2_LOG_UART_NUM, UART2_TXD_PIN, UART2_RXD_PIN, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE);
    uart_driver_install(UART2_LOG_UART_NUM, UART2_LOG_UART_RX_BUFFER_SIZE, 0, 0, NULL, 0);

    esp_log_set_vprintf(uart2_printf);
}
