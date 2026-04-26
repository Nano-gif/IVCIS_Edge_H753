#ifndef CONTROL_MANAGER_H
#define CONTROL_MANAGER_H

#include "shared_types.h"
#include <stdbool.h>
#include <stdint.h>

typedef enum {
  CONTROL_SRC_ETH = 0,
  CONTROL_SRC_RS485,
  CONTROL_SRC_LOCAL
} ControlSource_t;

typedef enum {
  CONTROL_CMD_ALARM = 0,
  CONTROL_CMD_SERVO
} ControlCommandType_t;

typedef struct {
  ControlCommandType_t type;
  ControlSource_t source;
  uint32_t tick_ms;
  union {
    struct {
      bool active;
    } alarm;
    struct {
      uint16_t angle_deg;
    } servo;
  } payload;
} ControlCommand_t;

bool Control_Manager_Init(void);
bool Control_Manager_Submit(const ControlCommand_t *cmd);
bool Control_Manager_SubmitAlarm(ControlSource_t source, bool active);
bool Control_Manager_SubmitServo(ControlSource_t source, uint16_t angle_deg);
bool Control_Manager_Receive(ControlCommand_t *cmd, uint32_t timeout_ms);

#endif /* CONTROL_MANAGER_H */
