#ifndef POWER_MANAGER_H
#define POWER_MANAGER_H

#include "shared_types.h"
#include <stdint.h>

/** 初始化功耗状态机, 默认 PWR_FULL */
void PowerMgr_Init(void);

/**
 * @brief  状态机主循环 (每帧调用一次)
 * @param  jpeg_size 当前 JPEG 帧大小 (DETECT 模式传 0)
 * @param  gray_motion 灰度帧差结果 (JPEG 模式传 false)
 * @retval 当前功耗模式
 */
PowerMode_t PowerMgr_Tick(uint32_t jpeg_size, bool gray_motion);

/** 获取当前模式对应的帧间隔(ms) */
uint32_t PowerMgr_GetDelay(void);

/** 获取当前功耗模式 */
PowerMode_t PowerMgr_GetMode(void);

#endif /* POWER_MANAGER_H */
