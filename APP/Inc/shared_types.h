#ifndef SHARED_TYPES_H
#define SHARED_TYPES_H

#include <stdbool.h>
#include <stdint.h>

/* 视觉模式枚举 */
typedef enum {
  VISION_MODE_JPEG = 0, /* JPEG 压缩 (抓拍上传) */
  VISION_MODE_GRAY      /* 160x120 灰度 (Y 通道提取) */
} VisionMode_t;

/* 功耗模式枚举 */
typedef enum {
  PWR_FULL = 0, /* JPEG 10fps, 上传+尺寸差检测 */
  PWR_LIGHT,    /* JPEG 2fps, 上传+尺寸差检测 */
  PWR_DETECT    /* Gray 1fps, 帧差检测, 不上传 */
} PowerMode_t;

/* 帧描述符: Vision -> Net 模块间传递 */
typedef struct {
  uint32_t frame_id; /* 单调递增计数 */
  uint32_t data_len; /* 实际数据长度 (字节) */
  uint8_t *p_data;   /* 指向 JPEG/Gray 缓冲区 (D2 SRAM) */
} FrameDesc_t;

/* UDP 分片包头 (每个 UDP 包都携带, 支持乱序重组) */
typedef struct __attribute__((packed)) {
  uint32_t magic; /* 0x49564349 ("IVCI") */
  uint32_t frame_id;
  uint16_t chunk_idx;  /* fragment index (0-based) */
  uint16_t chunk_cnt;  /* total fragment count */
  uint32_t total_size; /* total JPEG size in bytes */
  uint8_t flags;       /* bit0: low_quality, bit1: alarm_active */
  uint8_t reserved[3];
} NetChunkHdr_t; /* 20 bytes */

#define IVCIS_MAGIC 0x49564349U /* "IVCI" */

/* 云端指令类型 */
typedef enum { CMD_TYPE_VIOLATION = 0x01, CMD_TYPE_SERVO = 0x02 } CmdType_t;

/* 云端指令载荷 */
typedef struct {
  uint32_t frame_id;      /* 对应帧序号 */
  uint16_t angle;         /* 舵机目标角度 (0~180) */
  uint8_t cmd_type;       /* CmdType_t */
  uint8_t violation_flag; /* 1=检测到违章 */
} IVCIS_Command_t;

#endif /* SHARED_TYPES_H */
