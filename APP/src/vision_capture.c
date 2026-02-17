/**
 * @file    vision_capture.c
 * @brief   DCMI + DMA 底层采集 (force_stop / capture_one)
 */

#include "vision_capture.h"
#include "cmsis_os.h"
#include "debug_config.h"
#include "stm32h7xx_hal.h"

extern DCMI_HandleTypeDef hdcmi;
extern volatile uint8_t g_ov5640_frame_cplt;

#define FRAME_TIMEOUT_MS 2000

void Vision_ForceStop(void) {
  hdcmi.Instance->CR &= ~(DCMI_CR_CAPTURE | DCMI_CR_ENABLE);
  ((DMA_Stream_TypeDef *)hdcmi.DMA_Handle->Instance)->CR &= ~DMA_SxCR_EN;
  hdcmi.Instance->ICR = 0x1FU;
  hdcmi.State = HAL_DCMI_STATE_READY;
  hdcmi.DMA_Handle->State = HAL_DMA_STATE_READY;
  hdcmi.Lock = HAL_UNLOCKED;
  hdcmi.DMA_Handle->Lock = HAL_UNLOCKED;
}

uint8_t Vision_CaptureOne(uint8_t *buf, uint32_t word_len) {
  Vision_ForceStop();

  g_ov5640_frame_cplt = 0;
  HAL_DCMI_Start_DMA(&hdcmi, DCMI_MODE_SNAPSHOT, (uint32_t)buf, word_len);

  uint32_t t0 = HAL_GetTick();
  while (g_ov5640_frame_cplt == 0) {
    if ((HAL_GetTick() - t0) > FRAME_TIMEOUT_MS) {
      DBG_VISION("Capture timeout");
      Vision_ForceStop();
      return 0;
    }
    osDelay(1);
  }

  HAL_DCMI_Stop(&hdcmi);
  return 1;
}
