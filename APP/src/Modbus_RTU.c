#include "Modbus_RTU.h"
#include "Modbus_RegMap.h"
#include "app_config.h"
#include <stdbool.h>
#include <string.h>

static uint8_t s_slave_addr = MODBUS_DEFAULT_SLAVE_ADDR;

static uint16_t be16(const uint8_t *p) {
  return (uint16_t)(((uint16_t)p[0] << 8) | p[1]);
}

static void put_be16(uint8_t *p, uint16_t value) {
  p[0] = (uint8_t)(value >> 8);
  p[1] = (uint8_t)(value & 0xFFU);
}

static bool valid_slave_addr(uint8_t slave_addr) {
  return ((slave_addr >= 1U) && (slave_addr <= 247U));
}

static uint8_t reg_status_to_exception(ModbusRegStatus_t status) {
  switch (status) {
  case MODBUS_REG_ILLEGAL_ADDRESS:
    return MODBUS_EX_ILLEGAL_DATA_ADDRESS;
  case MODBUS_REG_ILLEGAL_VALUE:
    return MODBUS_EX_ILLEGAL_DATA_VALUE;
  case MODBUS_REG_DEVICE_FAILURE:
    return MODBUS_EX_SLAVE_DEVICE_FAILURE;
  case MODBUS_REG_OK:
  default:
    return MODBUS_EX_SLAVE_DEVICE_FAILURE;
  }
}

static size_t build_exception(uint8_t addr,
                              uint8_t func,
                              uint8_t ex,
                              uint8_t *response,
                              size_t response_max) {
  if ((response == NULL) || (response_max < 5U)) {
    return 0U;
  }

  response[0] = addr;
  response[1] = (uint8_t)(func | 0x80U);
  response[2] = ex;
  Modbus_RTU_AppendCrc(response, 3U);
  return 5U;
}

static size_t process_read_holding(const uint8_t *request,
                                   size_t request_len,
                                   uint8_t *response,
                                   size_t response_max,
                                   bool broadcast) {
  if (request_len != 8U) {
    return broadcast ? 0U : build_exception(request[0],
                                            request[1],
                                            MODBUS_EX_ILLEGAL_DATA_VALUE,
                                            response,
                                            response_max);
  }

  uint16_t start = be16(&request[2]);
  uint16_t qty = be16(&request[4]);
  if ((qty == 0U) || (qty > MODBUS_MAX_READ_REGS)) {
    return broadcast ? 0U : build_exception(request[0],
                                            request[1],
                                            MODBUS_EX_ILLEGAL_DATA_VALUE,
                                            response,
                                            response_max);
  }

  size_t response_len = 3U + ((size_t)qty * 2U) + 2U;
  if (response_max < response_len) {
    return 0U;
  }

  response[0] = request[0];
  response[1] = request[1];
  response[2] = (uint8_t)(qty * 2U);

  for (uint16_t i = 0; i < qty; i++) {
    uint16_t value = 0U;
    ModbusRegStatus_t status =
        Modbus_RegMap_ReadHolding((uint16_t)(start + i), &value);
    if (status != MODBUS_REG_OK) {
      return broadcast ? 0U : build_exception(request[0],
                                              request[1],
                                              reg_status_to_exception(status),
                                              response,
                                              response_max);
    }
    put_be16(&response[3U + ((size_t)i * 2U)], value);
  }

  Modbus_RTU_AppendCrc(response, response_len - 2U);
  return response_len;
}

static size_t process_read_discrete(const uint8_t *request,
                                    size_t request_len,
                                    uint8_t *response,
                                    size_t response_max,
                                    bool broadcast) {
  if (request_len != 8U) {
    return broadcast ? 0U : build_exception(request[0],
                                            request[1],
                                            MODBUS_EX_ILLEGAL_DATA_VALUE,
                                            response,
                                            response_max);
  }

  uint16_t start = be16(&request[2]);
  uint16_t qty = be16(&request[4]);
  if ((qty == 0U) || (qty > MODBUS_MAX_DISCRETE_INPUTS)) {
    return broadcast ? 0U : build_exception(request[0],
                                            request[1],
                                            MODBUS_EX_ILLEGAL_DATA_VALUE,
                                            response,
                                            response_max);
  }

  uint8_t byte_count = (uint8_t)((qty + 7U) / 8U);
  size_t response_len = 3U + byte_count + 2U;
  if (response_max < response_len) {
    return 0U;
  }

  response[0] = request[0];
  response[1] = request[1];
  response[2] = byte_count;
  memset(&response[3], 0, byte_count);

  for (uint16_t i = 0; i < qty; i++) {
    bool value = false;
    ModbusRegStatus_t status =
        Modbus_RegMap_ReadDiscrete((uint16_t)(start + i), &value);
    if (status != MODBUS_REG_OK) {
      return broadcast ? 0U : build_exception(request[0],
                                              request[1],
                                              reg_status_to_exception(status),
                                              response,
                                              response_max);
    }
    if (value) {
      response[3U + (i / 8U)] |= (uint8_t)(1U << (i % 8U));
    }
  }

  Modbus_RTU_AppendCrc(response, response_len - 2U);
  return response_len;
}

static void apply_slave_addr_side_effect(uint16_t addr, uint16_t value) {
  if ((addr == MODBUS_REG_CFG_SLAVE_ADDR) && valid_slave_addr((uint8_t)value)) {
    s_slave_addr = (uint8_t)value;
  }
}

static size_t process_write_single(const uint8_t *request,
                                   size_t request_len,
                                   uint8_t *response,
                                   size_t response_max,
                                   bool broadcast) {
  if (request_len != 8U) {
    return broadcast ? 0U : build_exception(request[0],
                                            request[1],
                                            MODBUS_EX_ILLEGAL_DATA_VALUE,
                                            response,
                                            response_max);
  }

  uint16_t addr = be16(&request[2]);
  uint16_t value = be16(&request[4]);
  ModbusRegStatus_t status = Modbus_RegMap_WriteHolding(addr, value);
  if (status != MODBUS_REG_OK) {
    return broadcast ? 0U : build_exception(request[0],
                                            request[1],
                                            reg_status_to_exception(status),
                                            response,
                                            response_max);
  }
  apply_slave_addr_side_effect(addr, value);

  if (broadcast) {
    return 0U;
  }

  if (response_max < 8U) {
    return 0U;
  }

  memcpy(response, request, 6U);
  Modbus_RTU_AppendCrc(response, 6U);
  return 8U;
}

static size_t process_write_multiple(const uint8_t *request,
                                     size_t request_len,
                                     uint8_t *response,
                                     size_t response_max,
                                     bool broadcast) {
  if (request_len < 9U) {
    return broadcast ? 0U : build_exception(request[0],
                                            request[1],
                                            MODBUS_EX_ILLEGAL_DATA_VALUE,
                                            response,
                                            response_max);
  }

  uint16_t start = be16(&request[2]);
  uint16_t qty = be16(&request[4]);
  uint8_t byte_count = request[6];
  if ((qty == 0U) || (qty > MODBUS_MAX_WRITE_REGS) ||
      (byte_count != (uint8_t)(qty * 2U)) ||
      (request_len != (size_t)(9U + byte_count))) {
    return broadcast ? 0U : build_exception(request[0],
                                            request[1],
                                            MODBUS_EX_ILLEGAL_DATA_VALUE,
                                            response,
                                            response_max);
  }

  uint16_t values[MODBUS_MAX_WRITE_REGS];
  for (uint16_t i = 0; i < qty; i++) {
    values[i] = be16(&request[7U + ((size_t)i * 2U)]);
    ModbusRegStatus_t status =
        Modbus_RegMap_ValidateWriteHolding((uint16_t)(start + i), values[i]);
    if (status != MODBUS_REG_OK) {
      return broadcast ? 0U : build_exception(request[0],
                                              request[1],
                                              reg_status_to_exception(status),
                                              response,
                                              response_max);
    }
  }

  for (uint16_t i = 0; i < qty; i++) {
    uint16_t addr = (uint16_t)(start + i);
    ModbusRegStatus_t status = Modbus_RegMap_WriteHolding(addr, values[i]);
    if (status != MODBUS_REG_OK) {
      return broadcast ? 0U : build_exception(request[0],
                                              request[1],
                                              reg_status_to_exception(status),
                                              response,
                                              response_max);
    }
    apply_slave_addr_side_effect(addr, values[i]);
  }

  if (broadcast) {
    return 0U;
  }

  if (response_max < 8U) {
    return 0U;
  }

  response[0] = request[0];
  response[1] = request[1];
  response[2] = request[2];
  response[3] = request[3];
  response[4] = request[4];
  response[5] = request[5];
  Modbus_RTU_AppendCrc(response, 6U);
  return 8U;
}

void Modbus_RTU_Init(uint8_t slave_addr) {
  s_slave_addr = valid_slave_addr(slave_addr) ? slave_addr
                                             : MODBUS_DEFAULT_SLAVE_ADDR;
}

uint8_t Modbus_RTU_GetSlaveAddr(void) {
  return s_slave_addr;
}

uint16_t Modbus_RTU_Crc16(const uint8_t *data, size_t len) {
  uint16_t crc = 0xFFFFU;
  if (data == NULL) {
    return crc;
  }

  for (size_t i = 0; i < len; i++) {
    crc ^= data[i];
    for (uint8_t bit = 0; bit < 8U; bit++) {
      if ((crc & 0x0001U) != 0U) {
        crc = (uint16_t)((crc >> 1) ^ 0xA001U);
      } else {
        crc >>= 1;
      }
    }
  }

  return crc;
}

void Modbus_RTU_AppendCrc(uint8_t *frame, size_t len_without_crc) {
  if (frame == NULL) {
    return;
  }

  uint16_t crc = Modbus_RTU_Crc16(frame, len_without_crc);
  frame[len_without_crc] = (uint8_t)(crc & 0xFFU);
  frame[len_without_crc + 1U] = (uint8_t)(crc >> 8);
}

size_t Modbus_RTU_GetExpectedLength(const uint8_t *data, size_t len) {
  if ((data == NULL) || (len < 2U)) {
    return 0U;
  }

  switch (data[1]) {
  case MODBUS_FC_READ_DISCRETE_INPUTS:
  case MODBUS_FC_READ_HOLDING_REGS:
  case MODBUS_FC_WRITE_SINGLE_REG:
    return 8U;
  case MODBUS_FC_WRITE_MULTIPLE_REGS:
    if (len < 7U) {
      return 0U;
    }
    return (size_t)9U + data[6];
  default:
    return 0U;
  }
}

size_t Modbus_RTU_ProcessFrame(const uint8_t *request,
                               size_t request_len,
                               uint8_t *response,
                               size_t response_max) {
  if ((request == NULL) || (response == NULL) || (request_len < 4U)) {
    return 0U;
  }

  uint16_t expected_crc = Modbus_RTU_Crc16(request, request_len - 2U);
  uint16_t rx_crc = (uint16_t)(request[request_len - 2U] |
                               ((uint16_t)request[request_len - 1U] << 8));
  if (expected_crc != rx_crc) {
    return 0U;
  }

  uint8_t addr = request[0];
  bool broadcast = (addr == 0U);
  if (!broadcast && (addr != s_slave_addr)) {
    return 0U;
  }

  switch (request[1]) {
  case MODBUS_FC_READ_DISCRETE_INPUTS:
    if (broadcast) {
      return 0U;
    }
    return process_read_discrete(request,
                                 request_len,
                                 response,
                                 response_max,
                                 broadcast);
  case MODBUS_FC_READ_HOLDING_REGS:
    if (broadcast) {
      return 0U;
    }
    return process_read_holding(request,
                                request_len,
                                response,
                                response_max,
                                broadcast);
  case MODBUS_FC_WRITE_SINGLE_REG:
    return process_write_single(request,
                                request_len,
                                response,
                                response_max,
                                broadcast);
  case MODBUS_FC_WRITE_MULTIPLE_REGS:
    return process_write_multiple(request,
                                  request_len,
                                  response,
                                  response_max,
                                  broadcast);
  default:
    return broadcast ? 0U : build_exception(addr,
                                            request[1],
                                            MODBUS_EX_ILLEGAL_FUNCTION,
                                            response,
                                            response_max);
  }
}
