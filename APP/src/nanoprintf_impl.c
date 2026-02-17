/**
 * @file    nanoprintf_impl.c
 * @brief   nanoprintf 实例化 + dbg_printf UART3 后端
 * @note    寄存器级 TX, 最小开销
 */

#include "stm32h7xx_hal.h"

/* nanoprintf: 在此编译单元生成实现 */
#define NANOPRINTF_IMPLEMENTATION
#define NANOPRINTF_USE_FIELD_WIDTH_FORMAT_SPECIFIERS 1
#define NANOPRINTF_USE_PRECISION_FORMAT_SPECIFIERS 1
#define NANOPRINTF_USE_FLOAT_FORMAT_SPECIFIERS 0
#define NANOPRINTF_USE_LARGE_FORMAT_SPECIFIERS 0
#define NANOPRINTF_USE_SMALL_FORMAT_SPECIFIERS 1
#define NANOPRINTF_USE_BINARY_FORMAT_SPECIFIERS 0
#define NANOPRINTF_USE_WRITEBACK_FORMAT_SPECIFIERS 0
#define NANOPRINTF_USE_ALT_FORM_FLAG 0
#include "nanoprintf.h"

#include <stdarg.h>

/* UART3 寄存器级单字符发送 */
static void uart3_putc(int32_t c, void *ctx) {
  (void)ctx;
  USART3->TDR = (uint8_t)c;
  while ((USART3->ISR & USART_ISR_TXE_TXFNF) == 0) {
    /* 等待发送就绪 */
  }
}

/**
 * @brief  格式化调试输出 (UART3)
 * @param  fmt 格式字符串 (ASCII)
 */
void dbg_printf(const char *fmt, ...) {
  if (fmt == NULL) {
    return;
  }
  va_list args;
  va_start(args, fmt);
  npf_vpprintf(uart3_putc, NULL, fmt, args);
  va_end(args);
}
