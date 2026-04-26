#include "RS485_Driver.h"
#include "RS485_Manager.h"
#include "app_config.h"
#include "debug_config.h"
#include "main.h"
#include "usart.h"

static volatile bool s_ready;
static volatile RS485_Direction_t s_direction = RS485_DIR_RX;
static volatile uint32_t s_rx_irq_count;
static volatile uint32_t s_error_count;
static uint8_t s_rx_byte;

static void rs485_de_gpio_init(void) {
  GPIO_InitTypeDef GPIO_InitStruct = {0};

  __HAL_RCC_GPIOD_CLK_ENABLE();
  HAL_GPIO_WritePin(RS485_DE_GPIO_Port, RS485_DE_Pin, GPIO_PIN_RESET);

  GPIO_InitStruct.Pin = RS485_DE_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(RS485_DE_GPIO_Port, &GPIO_InitStruct);
}

bool RS485_Driver_Init(void) {
  if (s_ready) {
    return true;
  }

  if (!RS485_Manager_Init()) {
    return false;
  }

  MX_USART2_UART_Init();
  rs485_de_gpio_init();
  RS485_Driver_SetDirection(RS485_DIR_RX);

  s_ready = (HAL_UART_Receive_IT(&huart2, &s_rx_byte, 1U) == HAL_OK);
  return s_ready;
}

bool RS485_Driver_IsReady(void) {
  return s_ready;
}

void RS485_Driver_SetDirection(RS485_Direction_t dir) {
  s_direction = dir;
  HAL_GPIO_WritePin(RS485_DE_GPIO_Port,
                    RS485_DE_Pin,
                    (dir == RS485_DIR_TX) ? GPIO_PIN_SET : GPIO_PIN_RESET);
}

RS485_Direction_t RS485_Driver_GetDirection(void) {
  return s_direction;
}

size_t RS485_Driver_Send(const uint8_t *data,
                         size_t len,
                         uint32_t timeout_ms) {
  if ((data == NULL) || (len == 0U) || (len > 0xFFFFU)) {
    return 0U;
  }

  if (!RS485_Driver_Init()) {
    return 0U;
  }

  RS485_Driver_SetDirection(RS485_DIR_TX);
  HAL_StatusTypeDef status = HAL_UART_Transmit(&huart2,
                                               (uint8_t *)data,
                                               (uint16_t)len,
                                               timeout_ms);
  RS485_Driver_SetDirection(RS485_DIR_RX);
  (void)HAL_UART_Receive_IT(&huart2, &s_rx_byte, 1U);

  return (status == HAL_OK) ? len : 0U;
}

size_t RS485_Driver_FlushTxStream(uint32_t timeout_ms) {
  uint8_t tx_buf[RS485_TASK_TX_BUF_SIZE];
  size_t total = 0U;

  for (;;) {
    size_t n = RS485_Manager_ReadTxBytes(tx_buf, sizeof(tx_buf), 0U);
    if (n == 0U) {
      break;
    }

    size_t sent = RS485_Driver_Send(tx_buf, n, timeout_ms);
    total += sent;
    if (sent != n) {
      break;
    }
  }

  return total;
}

uint32_t RS485_Driver_GetRxIrqCount(void) {
  return s_rx_irq_count;
}

uint32_t RS485_Driver_GetErrorCount(void) {
  return s_error_count;
}

void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart) {
  if ((huart == NULL) || (huart->Instance != USART2)) {
    return;
  }

  s_rx_irq_count++;
  (void)RS485_Manager_WriteRxBytesFromISR(&s_rx_byte, 1U);
  (void)HAL_UART_Receive_IT(&huart2, &s_rx_byte, 1U);
}

void HAL_UART_ErrorCallback(UART_HandleTypeDef *huart) {
  if ((huart == NULL) || (huart->Instance != USART2)) {
    return;
  }

  s_error_count++;
  RS485_Driver_SetDirection(RS485_DIR_RX);
  (void)HAL_UART_Receive_IT(&huart2, &s_rx_byte, 1U);
}
