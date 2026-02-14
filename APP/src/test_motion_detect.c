/**
 * @file    test_motion_detect.c
 * @brief   TDD: Gray mode switch + motion detection
 */

#include "Motion_Detect.h"
#include "Vision_Pipeline.h"
#include "app_config.h"
#include "debug_config.h"


void test_motion_detect_run(void) {
  DBG_INFO("======== TDD: Motion Detect ========");

  /* 1. Init */
  if (Vision_Init() != 0) {
    DBG_ERROR("[FAIL] Vision_Init");
    return;
  }
  DBG_INFO("[PASS] Vision_Init");

  /* 2. Switch to Gray */
  Vision_SetMode(VISION_MODE_GRAY);
  Motion_Init(CAM_GRAY_WIDTH, CAM_GRAY_HEIGHT);
  DBG_INFO("Switched to GRAY 160x120");

  /* 3. Discard first 3 frames (sensor stabilization) */
  for (uint8_t w = 0; w < 3; w++) {
    Vision_CaptureStart();
    HAL_Delay(200);
  }
  DBG_INFO("Warm-up done, starting detection");

  /* 4. Capture 10 frames */
  for (uint8_t i = 0; i < 10; i++) {
    Vision_CaptureStart();

    if (Vision_IsFrameReady()) {
      uint8_t *gray_ptr = Vision_GetFrameBuffer();

      /* Diagnostic: print first 8 pixel values */
      DBG_INFO("[F%d] px: %02X %02X %02X %02X %02X %02X %02X %02X", i,
               gray_ptr[0], gray_ptr[1], gray_ptr[2], gray_ptr[3], gray_ptr[4],
               gray_ptr[5], gray_ptr[6], gray_ptr[7]);

      bool motion = Motion_Detect(gray_ptr);
      uint32_t diff = Motion_GetDiff();

      DBG_INFO("[F%d] Diff=%lu Motion=%s", i, diff, motion ? "YES" : "no");
    } else {
      DBG_WARN("[F%d] Capture FAILED", i);
    }

    HAL_Delay(500);
  }

  /* 5. Switch back to JPEG */
  Vision_SetMode(VISION_MODE_JPEG);
  DBG_INFO("Switched back to JPEG");

  Vision_CaptureStart();
  if (Vision_IsFrameReady()) {
    DBG_INFO("[PASS] JPEG after mode switch OK, size=%lu",
             Vision_GetFrameSize());
  }

  DBG_INFO("======== TDD Done ========");
}
