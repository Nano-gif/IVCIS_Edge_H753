#include "Auto_Exposure.h"
#include "app_config.h"
#include "debug_config.h"
#include "ov5640.h"
#include <math.h>

static int8_t s_current_level = (int8_t)ATK_MC5640_EXPOSURE_LEVEL_5;

void AutoExp_Init(void) {
  s_current_level = (int8_t)ATK_MC5640_EXPOSURE_LEVEL_5;
  DBG_INFO("[AutoExp] Initialized at Level 5");
}

ImageQuality_t AutoExp_Analyze(uint8_t *frame) {
  ImageQuality_t q = {0};
  uint32_t sum = 0;
  uint32_t sq_sum = 0;
  float var = 0;

  /* 1. 计算平均亮度 (Brightness) */
  for (uint32_t i = 0; i < GRAY_FRAME_SIZE; i++) {
    sum += frame[i];
  }
  q.brightness = (float)sum / GRAY_FRAME_SIZE;

  /* 2. 计算清晰度 (基于相邻像素差值的方差 - 简化版 Laplacian) */
  /* 为效率跳过边界像素 */
  float edge_sum = 0;
  uint32_t count = 0;
  for (int y = 1; y < CAM_GRAY_HEIGHT - 1; y += 2) {
    for (int x = 1; x < CAM_GRAY_WIDTH - 1; x += 2) {
      int idx = y * CAM_GRAY_WIDTH + x;
      /* 简单的 拉普拉斯卷积算子 [0 -1 0; -1 4 -1; 0 -1 0] */
      int diff = (int)frame[idx] * 4 - (int)frame[idx - 1] -
                 (int)frame[idx + 1] - (int)frame[idx - CAM_GRAY_WIDTH] -
                 (int)frame[idx + CAM_GRAY_WIDTH];
      edge_sum += (float)(diff * diff);
      count++;
    }
  }
  q.sharpness = sqrtf(edge_sum / (float)count);

  /* 3. 综合判定 */
  q.is_low_quality = (q.sharpness < AE_SHARPNESS_THRESHOLD) ||
                     (fabsf(q.brightness - AE_TARGET_BRIGHTNESS) > 50.0f);

  return q;
}

void AutoExp_Adjust(ImageQuality_t *quality) {
  int8_t next_level = s_current_level;

  if (quality->brightness < (AE_TARGET_BRIGHTNESS - AE_BRIGHTNESS_TOLERANCE)) {
    /* 太暗 -> 增加曝光 */
    if (s_current_level < (int8_t)ATK_MC5640_EXPOSURE_LEVEL_10) {
      next_level++;
    }
  } else if (quality->brightness >
             (AE_TARGET_BRIGHTNESS + AE_BRIGHTNESS_TOLERANCE)) {
    /* 太亮 -> 减少曝光 */
    if (s_current_level > (int8_t)ATK_MC5640_EXPOSURE_LEVEL_0) {
      next_level--;
    }
  }

  if (next_level != s_current_level) {
    s_current_level = next_level;
    atk_mc5640_set_exposure_level((atk_mc5640_exposure_level_t)s_current_level);
    DBG_INFO("[AutoExp] Step Adjust -> Level %d (B:%.1f)", s_current_level,
             quality->brightness);
  }
}
