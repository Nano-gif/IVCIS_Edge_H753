#ifndef RS485_DRIVER_H
#define RS485_DRIVER_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

typedef enum {
  RS485_DIR_RX = 0,
  RS485_DIR_TX
} RS485_Direction_t;

bool RS485_Driver_Init(void);
bool RS485_Driver_IsReady(void);
void RS485_Driver_SetDirection(RS485_Direction_t dir);
RS485_Direction_t RS485_Driver_GetDirection(void);
size_t RS485_Driver_Send(const uint8_t *data, size_t len, uint32_t timeout_ms);
size_t RS485_Driver_FlushTxStream(uint32_t timeout_ms);
uint32_t RS485_Driver_GetRxIrqCount(void);
uint32_t RS485_Driver_GetErrorCount(void);

#endif /* RS485_DRIVER_H */
