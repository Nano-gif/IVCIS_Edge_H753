#ifndef MODBUS_REGMAP_H
#define MODBUS_REGMAP_H

#include <stdbool.h>
#include <stdint.h>

typedef enum {
  MODBUS_REG_OK = 0,
  MODBUS_REG_ILLEGAL_ADDRESS,
  MODBUS_REG_ILLEGAL_VALUE,
  MODBUS_REG_DEVICE_FAILURE
} ModbusRegStatus_t;

#define MODBUS_REG_CFG_SLAVE_ADDR 0x0030U

ModbusRegStatus_t Modbus_RegMap_ReadHolding(uint16_t addr, uint16_t *value);
ModbusRegStatus_t Modbus_RegMap_ValidateWriteHolding(uint16_t addr,
                                                     uint16_t value);
ModbusRegStatus_t Modbus_RegMap_WriteHolding(uint16_t addr, uint16_t value);
ModbusRegStatus_t Modbus_RegMap_ReadDiscrete(uint16_t addr, bool *value);

#endif /* MODBUS_REGMAP_H */
