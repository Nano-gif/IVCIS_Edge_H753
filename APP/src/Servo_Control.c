#include "Servo_Control.h"
#include "main.h"

static TIM_HandleTypeDef htim4;

void Servo_Init(void) {
  TIM_OC_InitTypeDef sConfigOC = {0};
  GPIO_InitTypeDef GPIO_InitStruct = {0};

  /* 1. 时钟使能 */
  __HAL_RCC_TIM4_CLK_ENABLE();
  __HAL_RCC_GPIOD_CLK_ENABLE();

  /* 2. GPIO 配置: PD15 -> TIM4_CH4 (AF2) */
  GPIO_InitStruct.Pin = GPIO_PIN_15;
  GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  GPIO_InitStruct.Alternate = GPIO_AF2_TIM4;
  HAL_GPIO_Init(GPIOD, &GPIO_InitStruct);

  /* 3. 定时器基础配置: 50Hz (20ms)
   * Timer Clock = 240MHz (APB1 * 2)
   * Prescaler = 4800-1 -> 50kHz clock
   * Period = 1000-1 -> 50Hz
   */
  htim4.Instance = TIM4;
  htim4.Init.Prescaler = 4800 - 1;
  htim4.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim4.Init.Period = 1000 - 1;
  htim4.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim4.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_ENABLE;
  HAL_TIM_PWM_Init(&htim4);

  /* 4. PWM 通道配置 */
  sConfigOC.OCMode = TIM_OCMODE_PWM1;
  sConfigOC.Pulse = 75; /* 默认 90° (1.5ms) */
  sConfigOC.OCPolarity = TIM_OCPOLARITY_HIGH;
  sConfigOC.OCFastMode = TIM_OCFAST_DISABLE;
  HAL_TIM_PWM_ConfigChannel(&htim4, &sConfigOC, TIM_CHANNEL_4);

  /* 5. 启动 PWM */
  HAL_TIM_PWM_Start(&htim4, TIM_CHANNEL_4);
}

void Servo_SetAngle(uint16_t angle) {
  if (angle > 180)
    angle = 180;

  /* 映射: 0° -> 0.5ms (25), 180° -> 2.5ms (125) */
  uint32_t pulse = 25 + (angle * 100 / 180);
  __HAL_TIM_SET_COMPARE(&htim4, TIM_CHANNEL_4, pulse);
}
