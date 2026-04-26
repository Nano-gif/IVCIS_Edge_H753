#include "RS485_Manager.h"
#include "FreeRTOS.h"
#include "app_config.h"
#include "cmsis_os.h"
#include "stream_buffer.h"
#include "task.h"

static StreamBufferHandle_t s_rs485_rx_stream;
static StreamBufferHandle_t s_rs485_tx_stream;

static TickType_t to_ticks(uint32_t timeout_ms) {
  if (timeout_ms == osWaitForever) {
    return portMAX_DELAY;
  }
  return pdMS_TO_TICKS(timeout_ms);
}

bool RS485_Manager_Init(void) {
  if ((s_rs485_rx_stream != NULL) && (s_rs485_tx_stream != NULL)) {
    return true;
  }

  if (s_rs485_rx_stream == NULL) {
    s_rs485_rx_stream = xStreamBufferCreate(RS485_RX_STREAM_SIZE,
                                            RS485_RX_TRIGGER_LEVEL);
  }
  if (s_rs485_tx_stream == NULL) {
    s_rs485_tx_stream = xStreamBufferCreate(RS485_TX_STREAM_SIZE,
                                            RS485_TX_TRIGGER_LEVEL);
  }
  return ((s_rs485_rx_stream != NULL) && (s_rs485_tx_stream != NULL));
}

size_t RS485_Manager_WriteRxBytes(const uint8_t *data,
                                  size_t len,
                                  uint32_t timeout_ms) {
  if ((data == NULL) || (len == 0U) || !RS485_Manager_Init()) {
    return 0U;
  }

  return xStreamBufferSend(s_rs485_rx_stream, data, len, to_ticks(timeout_ms));
}

size_t RS485_Manager_WriteRxBytesFromISR(const uint8_t *data, size_t len) {
  if ((data == NULL) || (len == 0U) || (s_rs485_rx_stream == NULL)) {
    return 0U;
  }

  BaseType_t higher_priority_task_woken = pdFALSE;
  size_t written = xStreamBufferSendFromISR(s_rs485_rx_stream,
                                            data,
                                            len,
                                            &higher_priority_task_woken);
  portYIELD_FROM_ISR(higher_priority_task_woken);
  return written;
}

size_t RS485_Manager_ReadRxBytes(uint8_t *data,
                                 size_t max_len,
                                 uint32_t timeout_ms) {
  if ((data == NULL) || (max_len == 0U) || !RS485_Manager_Init()) {
    return 0U;
  }

  return xStreamBufferReceive(s_rs485_rx_stream,
                              data,
                              max_len,
                              to_ticks(timeout_ms));
}

size_t RS485_Manager_WriteTxBytes(const uint8_t *data,
                                  size_t len,
                                  uint32_t timeout_ms) {
  if ((data == NULL) || (len == 0U) || !RS485_Manager_Init()) {
    return 0U;
  }

  return xStreamBufferSend(s_rs485_tx_stream, data, len, to_ticks(timeout_ms));
}

size_t RS485_Manager_ReadTxBytes(uint8_t *data,
                                 size_t max_len,
                                 uint32_t timeout_ms) {
  if ((data == NULL) || (max_len == 0U) || !RS485_Manager_Init()) {
    return 0U;
  }

  return xStreamBufferReceive(s_rs485_tx_stream,
                              data,
                              max_len,
                              to_ticks(timeout_ms));
}
