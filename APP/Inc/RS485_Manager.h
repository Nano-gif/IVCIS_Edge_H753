#ifndef RS485_MANAGER_H
#define RS485_MANAGER_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

bool RS485_Manager_Init(void);
size_t RS485_Manager_WriteRxBytes(const uint8_t *data,
                                  size_t len,
                                  uint32_t timeout_ms);
size_t RS485_Manager_WriteRxBytesFromISR(const uint8_t *data, size_t len);
size_t RS485_Manager_ReadRxBytes(uint8_t *data,
                                 size_t max_len,
                                 uint32_t timeout_ms);
size_t RS485_Manager_WriteTxBytes(const uint8_t *data,
                                  size_t len,
                                  uint32_t timeout_ms);
size_t RS485_Manager_ReadTxBytes(uint8_t *data,
                                 size_t max_len,
                                 uint32_t timeout_ms);

#endif /* RS485_MANAGER_H */
