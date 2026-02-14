#ifndef SHARED_TYPES_H
#define SHARED_TYPES_H

#include <stdbool.h>
#include <stdint.h>

/* Vision mode enumeration */
typedef enum {
  VISION_MODE_JPEG = 0, /* High quality JPEG encoding (snapshot) */
  VISION_MODE_GRAY      /* 160x120 Grayscale (Y-channel extraction) */
} VisionMode_t;

/* Frame descriptor: passed between Vision and Net modules */
typedef struct {
  uint32_t frame_id; /* Monotonically increasing counter */
  uint32_t data_len; /* Actual JPEG data length in bytes */
  uint8_t *p_data;   /* Pointer to JPEG/gray buffer (D2 SRAM) */
} FrameDesc_t;

/* Cloud command types */
typedef enum { CMD_TYPE_VIOLATION = 0x01, CMD_TYPE_SERVO = 0x02 } CmdType_t;

/* Cloud command payload */
typedef struct {
  uint32_t frame_id;      /* Which frame this result refers to */
  uint16_t angle;         /* Servo target angle (0~180), CMD_TYPE_SERVO */
  uint8_t cmd_type;       /* CmdType_t */
  uint8_t violation_flag; /* 1=violation detected, CMD_TYPE_VIOLATION */
} IVCIS_Command_t;

#endif /* SHARED_TYPES_H */
