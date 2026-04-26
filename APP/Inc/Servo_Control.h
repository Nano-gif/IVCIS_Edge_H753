#ifndef SERVO_CONTROL_H
#define SERVO_CONTROL_H

#include "shared_types.h"

/** 初始化舵机 PWM (TIM4_CH4 on PD15) */
void Servo_Init(void);

/**
 * 设置舵机角度 (0-180)
 * 映射为 0.5ms (0°) ~ 2.5ms (180°)
 */
void Servo_SetAngle(uint16_t angle);

#endif /* SERVO_CONTROL_H */
