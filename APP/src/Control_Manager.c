#include "Control_Manager.h"
#include "app_config.h"
#include "cmsis_os.h"
#include "stm32h7xx_hal.h"
#include <string.h>

static osMessageQueueId_t s_control_q;

bool Control_Manager_Init(void) {
  if (s_control_q != NULL) {
    return true;
  }

  s_control_q = osMessageQueueNew(CONTROL_COMMAND_QUEUE_DEPTH,
                                  sizeof(ControlCommand_t),
                                  NULL);
  return (s_control_q != NULL);
}

bool Control_Manager_Submit(const ControlCommand_t *cmd) {
  if ((cmd == NULL) || !Control_Manager_Init()) {
    return false;
  }

  ControlCommand_t stamped;
  memcpy(&stamped, cmd, sizeof(stamped));
  stamped.tick_ms = HAL_GetTick();

  return (osMessageQueuePut(s_control_q, &stamped, 0, 0) == osOK);
}

bool Control_Manager_SubmitAlarm(ControlSource_t source, bool active) {
  ControlCommand_t cmd = {
      .type = CONTROL_CMD_ALARM,
      .source = source,
      .payload.alarm.active = active,
  };
  return Control_Manager_Submit(&cmd);
}

bool Control_Manager_SubmitServo(ControlSource_t source, uint16_t angle_deg) {
  ControlCommand_t cmd = {
      .type = CONTROL_CMD_SERVO,
      .source = source,
      .payload.servo.angle_deg = angle_deg,
  };
  return Control_Manager_Submit(&cmd);
}

bool Control_Manager_Receive(ControlCommand_t *cmd, uint32_t timeout_ms) {
  if ((cmd == NULL) || !Control_Manager_Init()) {
    return false;
  }

  return (osMessageQueueGet(s_control_q, cmd, NULL, timeout_ms) == osOK);
}
