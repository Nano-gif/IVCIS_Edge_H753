#ifndef AUTO_EXPOSURE_H
#define AUTO_EXPOSURE_H

#include "shared_types.h"

/** 初始化自动曝光模块 */
void AutoExp_Init(void);

/**
 * 分析灰度帧质量
 * @param frame 指向 160x120 灰度数据的指针
 */
ImageQuality_t AutoExp_Analyze(uint8_t *frame);

/**
 * 根据分析结果调整 OV5640 曝光 (Safe Tweak 策略)
 */
void AutoExp_Adjust(ImageQuality_t *quality);

#endif /* AUTO_EXPOSURE_H */
