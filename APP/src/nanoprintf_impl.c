/**
 * @file    nanoprintf_impl.c
 * @brief   Nanoprintf implementation and UART redirection
 */

#define NANOPRINTF_USE_FIELD_WIDTH_FORMAT_SPECIFIERS 1
#define NANOPRINTF_USE_PRECISION_FORMAT_SPECIFIERS 1
#define NANOPRINTF_USE_FLOAT_FORMAT_SPECIFIERS 1
#define NANOPRINTF_USE_LARGE_FORMAT_SPECIFIERS 0
#define NANOPRINTF_USE_BINARY_FORMAT_SPECIFIERS 0
#define NANOPRINTF_USE_WRITEBACK_FORMAT_SPECIFIERS 0
#define NANOPRINTF_USE_SMALL_FORMAT_SPECIFIERS 0

// Compile the implementation
#define NANOPRINTF_IMPLEMENTATION
#include "nanoprintf.h"

#include "usart.h"

/* UART handle for debug output (defined in usart.c) */
extern UART_HandleTypeDef huart3;

/**
 * @brief  Single char output function for nanoprintf
 * @param  c   Character to send
 * @param  ctx Context pointer (unused)
 */
static void uart_putc(int c, void *ctx) {
  (void)ctx;
  /* Blocking send for debug reliability */
  HAL_UART_Transmit(&huart3, (uint8_t *)&c, 1, 100);
}

/**
 * @brief  Global debug printf implementation
 * @param  fmt Format string
 * @param  ... Variable arguments
 */
void dbg_printf(const char *fmt, ...) {
  va_list args;
  va_start(args, fmt);
  npf_vpprintf(uart_putc, NULL, fmt, args);
  va_end(args);
}
