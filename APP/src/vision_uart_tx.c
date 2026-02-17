/**
 * @file    vision_uart_tx.c
 * @brief   JPEG 帧 UART 发送 (XCAM 查看器)
 * @note    寄存器级 TX, 无 HAL 开销
 */

#include "Vision_Pipeline.h"
#include "stm32h7xx_hal.h"

void Vision_SendFrameUART(void) {
  uint8_t *buf = Vision_GetFrameBuffer();
  uint32_t len = Vision_GetFrameSize();

  if (buf == NULL || len == 0) {
    return;
  }

  /* 逐字节发送原始 JPEG 数据 (XCAM 依赖 SOI/EOI 标记) */
  for (uint32_t i = 0; i < len; i++) {
    USART3->TDR = buf[i];
    while ((USART3->ISR & USART_ISR_TXE_TXFNF) == 0) {
      /* 等待发送就绪 */
    }
  }
}
