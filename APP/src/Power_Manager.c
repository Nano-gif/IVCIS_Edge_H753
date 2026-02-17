/**
 * @file    Power_Manager.c
 * @brief   三级功耗状态机 (FULL / LIGHT / DETECT)
 *          FULL/LIGHT: JPEG 尺寸差检测
 *          DETECT:     灰度帧差检测
 */

#include "Power_Manager.h"
#include "app_config.h"
#include "debug_config.h"
#include "stm32h7xx_hal.h"

static PowerMode_t s_mode = PWR_FULL;
static uint32_t s_prev_size = 0;
static uint32_t s_quiet_start = 0; /* 无运动起始 tick */

static const uint32_t s_delay_lut[] = {PWR_FULL_DELAY_MS, PWR_LIGHT_DELAY_MS,
                                       PWR_DETECT_DELAY_MS};

/* JPEG 尺寸差判定 */
static bool jpeg_size_changed(uint32_t cur) {
  if (s_prev_size == 0) {
    s_prev_size = cur;
    return false;
  }
  uint32_t delta =
      (cur > s_prev_size) ? (cur - s_prev_size) : (s_prev_size - cur);
  s_prev_size = cur;
  return ((delta * 100) / s_prev_size) > PWR_JPEG_SIZE_DIFF_PCT;
}

/* 无运动持续秒数 */
static uint32_t quiet_seconds(void) {
  return (HAL_GetTick() - s_quiet_start) / 1000U;
}

static void switch_mode(PowerMode_t next) {
  DBG_INFO("[PWR] %d->%d", (int32_t)s_mode, (int32_t)next);
  s_mode = next;
  s_quiet_start = HAL_GetTick();
  s_prev_size = 0;
}

void PowerMgr_Init(void) {
  s_mode = PWR_FULL;
  s_prev_size = 0;
  s_quiet_start = HAL_GetTick();
  DBG_INFO("[PWR] Init FULL");
}

PowerMode_t PowerMgr_Tick(uint32_t jpeg_size, bool gray_motion) {
  bool motion = false;

  if (s_mode == PWR_DETECT) {
    motion = gray_motion;
  } else {
    motion = (jpeg_size > 0) ? jpeg_size_changed(jpeg_size) : false;
  }

  if (motion) {
    s_quiet_start = HAL_GetTick();
    if (s_mode != PWR_FULL) {
      switch_mode(PWR_FULL);
    }
    return s_mode;
  }

  /* 无运动降级 */
  uint32_t qs = quiet_seconds();
  if ((s_mode == PWR_FULL) && (qs >= PWR_FULL_TO_LIGHT_SEC)) {
    switch_mode(PWR_LIGHT);
  } else if ((s_mode == PWR_LIGHT) && (qs >= PWR_LIGHT_TO_DETECT_SEC)) {
    switch_mode(PWR_DETECT);
  }
  return s_mode;
}

uint32_t PowerMgr_GetDelay(void) { return s_delay_lut[s_mode]; }

PowerMode_t PowerMgr_GetMode(void) { return s_mode; }
