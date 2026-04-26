#ifndef VISION_PIPELINE_H
#define VISION_PIPELINE_H

#include "shared_types.h"
#include <stdint.h>

/* VisionMode_t, BufState_t 定义在 shared_types.h */

/** 初始化 OV5640 + DCMI, 默认 Gray 模式预热 */
int8_t Vision_Init(void);

/** 切换 JPEG / Gray 模式 (含 DCMI JPEGMode 动态切换) */
void Vision_SetMode(VisionMode_t mode);

/** 执行单帧 DCMI+DMA 采集 (同步等待) */
void Vision_CaptureStart(void);

/** 检查是否有完整帧就绪 */
uint8_t Vision_IsFrameReady(void);

/** 获取帧缓冲区指针 (无帧时返回 NULL) */
uint8_t *Vision_GetFrameBuffer(void);

/** 获取帧实际字节数 (无帧时返回 0) */
uint32_t Vision_GetFrameSize(void);

/** 获取当前活跃缓冲区索引 (0=A, 1=B) */
uint8_t Vision_GetActiveIdx(void);

/** 设置缓冲区状态 */
void Vision_SetBufState(uint8_t idx, BufState_t state);

/** 获取缓冲区状态 */
BufState_t Vision_GetBufState(uint8_t idx);

/** 切换到下一个 FREE 缓冲区, 成功返回 1, 背压返回 0 */
uint8_t Vision_SwitchBuffer(void);

#endif /* VISION_PIPELINE_H */
