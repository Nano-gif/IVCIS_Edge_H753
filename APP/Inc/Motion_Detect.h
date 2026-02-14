#ifndef MOTION_DETECT_H
#define MOTION_DETECT_H

#include <stdbool.h>
#include <stdint.h>

/**
 * @brief 初始化运动检测模块
 * @param width  图像宽度
 * @param height 图像高度
 */
void Motion_Init(uint16_t width, uint16_t height);

/**
 * @brief 运行帧差法运动检测
 * @param current_frame 当前灰度帧指针 (8-bit)
 * @retval true  检测到运动
 * @retval false 未检测到运动
 */
bool Motion_Detect(uint8_t *current_frame);

/**
 * @brief 获取上一次帧间平均差异值
 */
uint32_t Motion_GetDiff(void);

#endif /* MOTION_DETECT_H */
