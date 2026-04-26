#include "Alarm_Handler.h"
#include "main.h"

void Alarm_Init(void) {
  /* 初始化红灯 (LD3) 为熄灭状态 */
  HAL_GPIO_WritePin(LD3_GPIO_Port, LD3_Pin, GPIO_PIN_RESET);
}

void Alarm_Set(bool active) {
  HAL_GPIO_WritePin(LD3_GPIO_Port, LD3_Pin,
                    active ? GPIO_PIN_SET : GPIO_PIN_RESET);
}
