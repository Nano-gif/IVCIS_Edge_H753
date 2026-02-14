/**
 * @file    Motion_Detect.c
 * @brief   帧差法运动检测 (160x120 灰度帧)
 */

#include "Motion_Detect.h"
#include "app_config.h"
#include <string.h>

/* 上一帧灰度缓冲 (静态分配，无堆) */
static uint8_t s_prev_frame[GRAY_FRAME_SIZE];
static bool s_initialized = false;
static uint32_t s_last_diff = 0;

/* 可调阈值 */
#define MOTION_THRESHOLD 15   /* 平均像素强度变化阈值 */
#define MOTION_MIN_PIXELS 500 /* 最小显著变化像素数 */
#define PIXEL_SENSITIVITY 20  /* 单像素灵敏度阈值 */

void Motion_Init(uint16_t width, uint16_t height) {
  (void)width;
  (void)height;
  memset(s_prev_frame, 0, sizeof(s_prev_frame));
  s_initialized = false;
  s_last_diff = 0;
}

bool Motion_Detect(uint8_t *current_frame) {
  if (current_frame == NULL) {
    return false;
  }

  /* 首帧仅存储，不做检测 */
  if (!s_initialized) {
    memcpy(s_prev_frame, current_frame, GRAY_FRAME_SIZE);
    s_initialized = true;
    return false;
  }

  uint32_t total_diff = 0;
  uint32_t changed_pixels = 0;

  for (uint32_t i = 0; i < GRAY_FRAME_SIZE; i++) {
    /* 使用 int16_t 防止 uint8_t 减法下溢 */
    int16_t diff = (int16_t)current_frame[i] - (int16_t)s_prev_frame[i];
    uint8_t abs_diff = (uint8_t)((diff < 0) ? (-diff) : diff);

    total_diff += abs_diff;
    if (abs_diff > PIXEL_SENSITIVITY) {
      changed_pixels++;
    }
  }

  s_last_diff = total_diff / GRAY_FRAME_SIZE;

  /* 更新上一帧 */
  memcpy(s_prev_frame, current_frame, GRAY_FRAME_SIZE);

  /* 判定逻辑: 平均差异 或 变化像素数超阈值 */
  if ((s_last_diff > MOTION_THRESHOLD) ||
      (changed_pixels > MOTION_MIN_PIXELS)) {
    return true;
  }

  return false;
}

uint32_t Motion_GetDiff(void) { return s_last_diff; }
