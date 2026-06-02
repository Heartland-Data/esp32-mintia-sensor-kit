/**
 * @file uart_log.h
 * @brief UART2を用いたログ出力機能のヘッダファイル
 * @copyright Copyright (C) 2026, Heartland Data inc. All Rights Reserved.
 * SPDX-FileCopyrightText: 2026 Heartland Data inc.
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef UART_LOG_H
#define UART_LOG_H

#ifdef __cplusplus
extern "C"
{
#endif

#include <stdarg.h>

// UART2 TX/RXピンとボーレート
#define UART2_LOG_UART_NUM (UART_NUM_2)
#define UART2_TXD_PIN (17) // GPIO_NUM_17
#define UART2_RXD_PIN (16) // GPIO_NUM_16
#define UART2_BAUD_RATE (115200)

    int uart2_printf(const char *format, va_list args);
    void init_uart2_for_log(void);

#ifdef __cplusplus
}
#endif

#endif // UART_LOG_H
