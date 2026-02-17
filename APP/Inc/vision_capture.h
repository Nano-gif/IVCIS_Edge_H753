#ifndef VISION_CAPTURE_H
#define VISION_CAPTURE_H

#include <stdint.h>

/** 寄存器级强制停止 DCMI+DMA */
void Vision_ForceStop(void);

/**
 * @brief  单帧 DMA 采集 (RTOS 友好等待)
 * @param  buf  目标缓冲区
 * @param  word_len DMA 传输长度 (32位字)
 * @retval 1=成功, 0=超时
 */
uint8_t Vision_CaptureOne(uint8_t *buf, uint32_t word_len);

#endif /* VISION_CAPTURE_H */
