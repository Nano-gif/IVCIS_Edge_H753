#include "Modbus_RegMap.h"
#include "Camera_Task.h"
#include "Control_Manager.h"
#include "Net_Client.h"
#include "Power_Manager.h"
#include "Radar_Manager.h"
#include "app_config.h"
#include "stm32h7xx_hal.h"

#define REG_DEVICE_ID 0x0000U
#define REG_FW_VER_MAJOR 0x0001U
#define REG_FW_VER_MINOR 0x0002U
#define REG_UPTIME_H 0x0003U
#define REG_UPTIME_L 0x0004U

#define REG_POWER_MODE 0x0010U
#define REG_NET_STATE 0x0011U
#define REG_FRAME_CNT_H 0x0012U
#define REG_FRAME_CNT_L 0x0013U
#define REG_TX_CNT_H 0x0014U
#define REG_TX_CNT_L 0x0015U
#define REG_BRIGHTNESS 0x0016U
#define REG_SHARPNESS 0x0017U
#define REG_JPEG_SIZE_H 0x0018U
#define REG_JPEG_SIZE_L 0x0019U
#define REG_RADAR_STATE 0x001AU
#define REG_RADAR_TARGET_COUNT 0x001BU
#define REG_RADAR_RANGE_CM 0x001CU
#define REG_RADAR_SPEED_CMS 0x001DU
#define REG_RADAR_LANE_ID 0x001EU
#define REG_FUSION_STATE 0x001FU

#define REG_CMD_TRIGGER 0x0020U
#define REG_CMD_POWER_MODE 0x0021U
#define REG_CMD_SERVO_ANGLE 0x0022U
#define REG_CMD_ALARM 0x0023U
#define REG_CMD_REBOOT 0x0024U
#define REG_CMD_RADAR_ENABLE 0x0025U
#define REG_CMD_CAPTURE_ZONE 0x0026U

#define REG_CFG_BAUDRATE 0x0031U
#define REG_CFG_DEST_IP_H 0x0032U
#define REG_CFG_DEST_IP_L 0x0033U
#define REG_CFG_DEST_PORT 0x0034U
#define REG_CFG_JPEG_FPS 0x0035U
#define REG_CFG_FULL_TIMEOUT 0x0036U
#define REG_CFG_LIGHT_TIMEOUT 0x0037U
#define REG_CFG_AE_TARGET 0x0038U
#define REG_CFG_RADAR_TIMEOUT_MS 0x0039U
#define REG_CFG_CAPTURE_RANGE_CM 0x003AU
#define REG_CFG_RADAR_MIN_CONF 0x003BU

static uint16_t s_cmd_power_mode = 0x00FFU;
static uint16_t s_cmd_servo_angle = 90U;
static uint16_t s_cmd_alarm = 0U;
static uint16_t s_cmd_radar_enable = 1U;
static uint16_t s_cmd_capture_zone = RADAR_CAPTURE_MAX_RANGE_CM;

static uint16_t s_cfg_slave_addr = MODBUS_DEFAULT_SLAVE_ADDR;
static uint16_t s_cfg_baudrate = 0U;
static uint16_t s_cfg_dest_ip_h =
    ((uint16_t)DEST_IP_ADDR0 << 8) | (uint16_t)DEST_IP_ADDR1;
static uint16_t s_cfg_dest_ip_l =
    ((uint16_t)DEST_IP_ADDR2 << 8) | (uint16_t)DEST_IP_ADDR3;
static uint16_t s_cfg_dest_port = UDP_REMOTE_PORT;
static uint16_t s_cfg_jpeg_fps = 10U;
static uint16_t s_cfg_full_timeout = PWR_FULL_TO_LIGHT_SEC;
static uint16_t s_cfg_light_timeout = PWR_LIGHT_TO_DETECT_SEC;
static uint16_t s_cfg_ae_target = 128U;
static uint16_t s_cfg_radar_timeout_ms = RADAR_TARGET_TIMEOUT_MS;
static uint16_t s_cfg_capture_range_cm = RADAR_CAPTURE_MAX_RANGE_CM;
static uint16_t s_cfg_radar_min_conf = RADAR_MIN_CONFIDENCE;

static uint16_t u32_high(uint32_t value) {
  return (uint16_t)(value >> 16);
}

static uint16_t u32_low(uint32_t value) {
  return (uint16_t)(value & 0xFFFFU);
}

static bool radar_in_capture_zone(const RadarSummary_t *summary) {
  if ((summary == NULL) || !summary->valid || (summary->target_count == 0U)) {
    return false;
  }

  if (summary->confidence < s_cfg_radar_min_conf) {
    return false;
  }

  if ((summary->range_cm < RADAR_CAPTURE_MIN_RANGE_CM) ||
      (summary->range_cm > s_cfg_capture_range_cm)) {
    return false;
  }

  return (summary->direction != RADAR_DIR_LEAVING);
}

static uint16_t radar_state_value(const RadarSummary_t *summary) {
  if (Radar_IsTimedOut()) {
    return 3U;
  }

  if ((summary != NULL) && summary->valid && (summary->target_count > 0U)) {
    return 2U;
  }

  return 1U;
}

ModbusRegStatus_t Modbus_RegMap_ReadHolding(uint16_t addr, uint16_t *value) {
  if (value == NULL) {
    return MODBUS_REG_DEVICE_FAILURE;
  }

  uint32_t seconds = HAL_GetTick() / 1000U;
  uint32_t tx_cnt = Net_Client_GetTxCount();
  RadarSummary_t radar = {0};
  (void)Radar_GetSummaryCopy(&radar);

  switch (addr) {
  case REG_DEVICE_ID:
    *value = 0x4956U;
    return MODBUS_REG_OK;
  case REG_FW_VER_MAJOR:
    *value = 1U;
    return MODBUS_REG_OK;
  case REG_FW_VER_MINOR:
    *value = 0U;
    return MODBUS_REG_OK;
  case REG_UPTIME_H:
    *value = u32_high(seconds);
    return MODBUS_REG_OK;
  case REG_UPTIME_L:
    *value = u32_low(seconds);
    return MODBUS_REG_OK;
  case REG_POWER_MODE:
    *value = (uint16_t)PowerMgr_GetMode();
    return MODBUS_REG_OK;
  case REG_NET_STATE:
    *value = (uint16_t)Net_Client_GetTxState();
    return MODBUS_REG_OK;
  case REG_FRAME_CNT_H:
  case REG_FRAME_CNT_L:
  case REG_BRIGHTNESS:
  case REG_SHARPNESS:
  case REG_JPEG_SIZE_H:
  case REG_JPEG_SIZE_L:
    *value = 0U;
    return MODBUS_REG_OK;
  case REG_TX_CNT_H:
    *value = u32_high(tx_cnt);
    return MODBUS_REG_OK;
  case REG_TX_CNT_L:
    *value = u32_low(tx_cnt);
    return MODBUS_REG_OK;
  case REG_RADAR_STATE:
    *value = radar_state_value(&radar);
    return MODBUS_REG_OK;
  case REG_RADAR_TARGET_COUNT:
    *value = radar.target_count;
    return MODBUS_REG_OK;
  case REG_RADAR_RANGE_CM:
    *value = radar.range_cm;
    return MODBUS_REG_OK;
  case REG_RADAR_SPEED_CMS:
    *value = (uint16_t)radar.speed_cms;
    return MODBUS_REG_OK;
  case REG_RADAR_LANE_ID:
    *value = radar.lane_id;
    return MODBUS_REG_OK;
  case REG_FUSION_STATE:
    *value = radar_in_capture_zone(&radar) ? 2U : 0U;
    return MODBUS_REG_OK;
  case REG_CMD_TRIGGER:
    *value = 0U;
    return MODBUS_REG_OK;
  case REG_CMD_POWER_MODE:
    *value = s_cmd_power_mode;
    return MODBUS_REG_OK;
  case REG_CMD_SERVO_ANGLE:
    *value = s_cmd_servo_angle;
    return MODBUS_REG_OK;
  case REG_CMD_ALARM:
    *value = s_cmd_alarm;
    return MODBUS_REG_OK;
  case REG_CMD_REBOOT:
    *value = 0U;
    return MODBUS_REG_OK;
  case REG_CMD_RADAR_ENABLE:
    *value = s_cmd_radar_enable;
    return MODBUS_REG_OK;
  case REG_CMD_CAPTURE_ZONE:
    *value = s_cmd_capture_zone;
    return MODBUS_REG_OK;
  case MODBUS_REG_CFG_SLAVE_ADDR:
    *value = s_cfg_slave_addr;
    return MODBUS_REG_OK;
  case REG_CFG_BAUDRATE:
    *value = s_cfg_baudrate;
    return MODBUS_REG_OK;
  case REG_CFG_DEST_IP_H:
    *value = s_cfg_dest_ip_h;
    return MODBUS_REG_OK;
  case REG_CFG_DEST_IP_L:
    *value = s_cfg_dest_ip_l;
    return MODBUS_REG_OK;
  case REG_CFG_DEST_PORT:
    *value = s_cfg_dest_port;
    return MODBUS_REG_OK;
  case REG_CFG_JPEG_FPS:
    *value = s_cfg_jpeg_fps;
    return MODBUS_REG_OK;
  case REG_CFG_FULL_TIMEOUT:
    *value = s_cfg_full_timeout;
    return MODBUS_REG_OK;
  case REG_CFG_LIGHT_TIMEOUT:
    *value = s_cfg_light_timeout;
    return MODBUS_REG_OK;
  case REG_CFG_AE_TARGET:
    *value = s_cfg_ae_target;
    return MODBUS_REG_OK;
  case REG_CFG_RADAR_TIMEOUT_MS:
    *value = s_cfg_radar_timeout_ms;
    return MODBUS_REG_OK;
  case REG_CFG_CAPTURE_RANGE_CM:
    *value = s_cfg_capture_range_cm;
    return MODBUS_REG_OK;
  case REG_CFG_RADAR_MIN_CONF:
    *value = s_cfg_radar_min_conf;
    return MODBUS_REG_OK;
  default:
    return MODBUS_REG_ILLEGAL_ADDRESS;
  }
}

ModbusRegStatus_t Modbus_RegMap_ValidateWriteHolding(uint16_t addr,
                                                     uint16_t value) {
  switch (addr) {
  case REG_CMD_TRIGGER:
    return (value <= 1U) ? MODBUS_REG_OK : MODBUS_REG_ILLEGAL_VALUE;
  case REG_CMD_POWER_MODE:
    return ((value <= PWR_DETECT) || (value == 0x00FFU))
               ? MODBUS_REG_OK
               : MODBUS_REG_ILLEGAL_VALUE;
  case REG_CMD_SERVO_ANGLE:
    return (value <= 180U) ? MODBUS_REG_OK : MODBUS_REG_ILLEGAL_VALUE;
  case REG_CMD_ALARM:
  case REG_CMD_RADAR_ENABLE:
    return (value <= 1U) ? MODBUS_REG_OK : MODBUS_REG_ILLEGAL_VALUE;
  case REG_CMD_REBOOT:
    return (value == 0xA55AU) ? MODBUS_REG_OK : MODBUS_REG_ILLEGAL_VALUE;
  case REG_CMD_CAPTURE_ZONE:
    return MODBUS_REG_OK;
  case MODBUS_REG_CFG_SLAVE_ADDR:
    return ((value >= 1U) && (value <= 247U)) ? MODBUS_REG_OK
                                              : MODBUS_REG_ILLEGAL_VALUE;
  case REG_CFG_BAUDRATE:
    return (value <= 3U) ? MODBUS_REG_OK : MODBUS_REG_ILLEGAL_VALUE;
  case REG_CFG_DEST_IP_H:
  case REG_CFG_DEST_IP_L:
    return MODBUS_REG_OK;
  case REG_CFG_DEST_PORT:
    return (value != 0U) ? MODBUS_REG_OK : MODBUS_REG_ILLEGAL_VALUE;
  case REG_CFG_JPEG_FPS:
    return ((value >= 1U) && (value <= 30U)) ? MODBUS_REG_OK
                                             : MODBUS_REG_ILLEGAL_VALUE;
  case REG_CFG_FULL_TIMEOUT:
  case REG_CFG_LIGHT_TIMEOUT:
  case REG_CFG_RADAR_TIMEOUT_MS:
  case REG_CFG_CAPTURE_RANGE_CM:
    return (value != 0U) ? MODBUS_REG_OK : MODBUS_REG_ILLEGAL_VALUE;
  case REG_CFG_AE_TARGET:
    return (value <= 255U) ? MODBUS_REG_OK : MODBUS_REG_ILLEGAL_VALUE;
  case REG_CFG_RADAR_MIN_CONF:
    return (value <= 100U) ? MODBUS_REG_OK : MODBUS_REG_ILLEGAL_VALUE;
  default:
    return MODBUS_REG_ILLEGAL_ADDRESS;
  }
}

ModbusRegStatus_t Modbus_RegMap_WriteHolding(uint16_t addr, uint16_t value) {
  ModbusRegStatus_t status = Modbus_RegMap_ValidateWriteHolding(addr, value);
  if (status != MODBUS_REG_OK) {
    return status;
  }

  switch (addr) {
  case REG_CMD_TRIGGER:
    if ((value == 1U) && !Camera_RequestCapture()) {
      return MODBUS_REG_DEVICE_FAILURE;
    }
    return MODBUS_REG_OK;
  case REG_CMD_POWER_MODE:
    s_cmd_power_mode = value;
    return MODBUS_REG_OK;
  case REG_CMD_SERVO_ANGLE:
    s_cmd_servo_angle = value;
    return Control_Manager_SubmitServo(CONTROL_SRC_RS485, value)
               ? MODBUS_REG_OK
               : MODBUS_REG_DEVICE_FAILURE;
  case REG_CMD_ALARM:
    s_cmd_alarm = value;
    return Control_Manager_SubmitAlarm(CONTROL_SRC_RS485, value != 0U)
               ? MODBUS_REG_OK
               : MODBUS_REG_DEVICE_FAILURE;
  case REG_CMD_REBOOT:
    return MODBUS_REG_OK;
  case REG_CMD_RADAR_ENABLE:
    s_cmd_radar_enable = value;
    return MODBUS_REG_OK;
  case REG_CMD_CAPTURE_ZONE:
    s_cmd_capture_zone = value;
    return MODBUS_REG_OK;
  case MODBUS_REG_CFG_SLAVE_ADDR:
    s_cfg_slave_addr = value;
    return MODBUS_REG_OK;
  case REG_CFG_BAUDRATE:
    s_cfg_baudrate = value;
    return MODBUS_REG_OK;
  case REG_CFG_DEST_IP_H:
    s_cfg_dest_ip_h = value;
    return MODBUS_REG_OK;
  case REG_CFG_DEST_IP_L:
    s_cfg_dest_ip_l = value;
    return MODBUS_REG_OK;
  case REG_CFG_DEST_PORT:
    s_cfg_dest_port = value;
    return MODBUS_REG_OK;
  case REG_CFG_JPEG_FPS:
    s_cfg_jpeg_fps = value;
    return MODBUS_REG_OK;
  case REG_CFG_FULL_TIMEOUT:
    s_cfg_full_timeout = value;
    return MODBUS_REG_OK;
  case REG_CFG_LIGHT_TIMEOUT:
    s_cfg_light_timeout = value;
    return MODBUS_REG_OK;
  case REG_CFG_AE_TARGET:
    s_cfg_ae_target = value;
    return MODBUS_REG_OK;
  case REG_CFG_RADAR_TIMEOUT_MS:
    s_cfg_radar_timeout_ms = value;
    return MODBUS_REG_OK;
  case REG_CFG_CAPTURE_RANGE_CM:
    s_cfg_capture_range_cm = value;
    return MODBUS_REG_OK;
  case REG_CFG_RADAR_MIN_CONF:
    s_cfg_radar_min_conf = value;
    return MODBUS_REG_OK;
  default:
    return MODBUS_REG_ILLEGAL_ADDRESS;
  }
}

ModbusRegStatus_t Modbus_RegMap_ReadDiscrete(uint16_t addr, bool *value) {
  if (value == NULL) {
    return MODBUS_REG_DEVICE_FAILURE;
  }

  RadarSummary_t radar = {0};
  (void)Radar_GetSummaryCopy(&radar);
  bool radar_present = radar.valid && (radar.target_count > 0U);

  switch (addr) {
  case 0x0000U:
    *value = false;
    return MODBUS_REG_OK;
  case 0x0001U:
    *value = true;
    return MODBUS_REG_OK;
  case 0x0002U:
    *value = (Net_Client_GetTxState() == NET_READY) ||
             (Net_Client_GetTxState() == NET_SENDING);
    return MODBUS_REG_OK;
  case 0x0003U:
    *value = false;
    return MODBUS_REG_OK;
  case 0x0004U:
    *value = (s_cmd_alarm != 0U);
    return MODBUS_REG_OK;
  case 0x0005U:
    *value = false;
    return MODBUS_REG_OK;
  case 0x0006U:
    *value = radar_present;
    return MODBUS_REG_OK;
  case 0x0007U:
    *value = radar_in_capture_zone(&radar);
    return MODBUS_REG_OK;
  case 0x0008U:
    *value = Radar_IsTimedOut();
    return MODBUS_REG_OK;
  case 0x0009U:
    *value = radar_in_capture_zone(&radar);
    return MODBUS_REG_OK;
  default:
    return MODBUS_REG_ILLEGAL_ADDRESS;
  }
}
