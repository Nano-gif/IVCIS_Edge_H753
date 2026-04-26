#ifndef MODBUS_RTU_H
#define MODBUS_RTU_H

#include <stddef.h>
#include <stdint.h>

#define MODBUS_FC_READ_DISCRETE_INPUTS 0x02U
#define MODBUS_FC_READ_HOLDING_REGS 0x03U
#define MODBUS_FC_WRITE_SINGLE_REG 0x06U
#define MODBUS_FC_WRITE_MULTIPLE_REGS 0x10U

#define MODBUS_EX_ILLEGAL_FUNCTION 0x01U
#define MODBUS_EX_ILLEGAL_DATA_ADDRESS 0x02U
#define MODBUS_EX_ILLEGAL_DATA_VALUE 0x03U
#define MODBUS_EX_SLAVE_DEVICE_FAILURE 0x04U

void Modbus_RTU_Init(uint8_t slave_addr);
uint8_t Modbus_RTU_GetSlaveAddr(void);
uint16_t Modbus_RTU_Crc16(const uint8_t *data, size_t len);
void Modbus_RTU_AppendCrc(uint8_t *frame, size_t len_without_crc);
size_t Modbus_RTU_GetExpectedLength(const uint8_t *data, size_t len);
size_t Modbus_RTU_ProcessFrame(const uint8_t *request,
                               size_t request_len,
                               uint8_t *response,
                               size_t response_max);

#endif /* MODBUS_RTU_H */
