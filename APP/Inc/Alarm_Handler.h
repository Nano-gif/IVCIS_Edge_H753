#ifndef ALARM_HANDLER_H
#define ALARM_HANDLER_H

#include "shared_types.h"

/** 初始化报警模块 */
void Alarm_Init(void);

/**
 * 触发或停止报警
 * @param active true 为开启红灯警报，false 为关闭
 */
void Alarm_Set(bool active);

#endif /* ALARM_HANDLER_H */
