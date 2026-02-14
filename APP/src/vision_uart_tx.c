/**
 * @file    vision_uart_tx.c
 * @brief   Send JPEG frame over UART for ATK-XCAM viewer
 * @note    Uses register-level UART TX (same as reference example)
 */

#include "Vision_Pipeline.h"
#include "debug_config.h"
#include "stm32h7xx_hal.h"

extern UART_HandleTypeDef huart3;

void Vision_SendFrameUART(void) {
  uint8_t *buf = Vision_GetFrameBuffer();
  uint32_t len = Vision_GetFrameSize();

  if (buf == NULL || len == 0) {
    return;
  }

  /* Send raw JPEG via register (reliable, no HAL overhead) */
  for (uint32_t i = 0; i < len; i++) {
    USART3->TDR = buf[i];
    while ((USART3->ISR & USART_ISR_TXE_TXFNF) == 0)
      ;
  }
}
