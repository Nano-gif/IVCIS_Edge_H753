/**
 * @file    test_mode_switch.c
 * @brief   TDD: JPEG <-> GRAY mode switch stability
 *
 * Validates Vision_SetMode() can switch between JPEG/GRAY.
 * JPEG captures use retry to tolerate DuPont wire noise.
 * Acceptance: all rounds pass (JPEG within retries).
 */

#include "Vision_Pipeline.h"
#include "app_config.h"
#include "debug_config.h"
#include "ov5640.h"
#include "stm32h7xx_hal.h"

#define SWITCH_ROUNDS 5
#define JPEG_RETRIES 3
#define JPEG_SETTLE_MS 800

static uint8_t s_pass = 0;
static uint8_t s_fail = 0;

static void chk(uint8_t cond, const char *msg) {
  if (cond) {
    s_pass++;
  } else {
    s_fail++;
  }
  dbg_printf("[%s] %s\r\n", cond ? "PASS" : "FAIL", msg);
}

/* JPEG: retry up to JPEG_RETRIES (DuPont wire noise tolerance) */
static void test_jpeg_capture(uint8_t round) {
  Vision_SetMode(VISION_MODE_JPEG);
  OV5640_Apply_Best_Settings();
  HAL_Delay(JPEG_SETTLE_MS);

  uint8_t ok = 0;
  for (uint8_t r = 0; r < JPEG_RETRIES; r++) {
    Vision_CaptureStart();
    if (Vision_IsFrameReady()) {
      dbg_printf("  R%u JPEG: sz=%lu (try %u)\r\n", round,
                 (unsigned long)Vision_GetFrameSize(), r + 1);
      ok = 1;
      break;
    }
    dbg_printf("  R%u JPEG retry %u\r\n", round, r + 1);
  }
  chk(ok, "JPEG capture OK (with retry)");
}

/* GRAY: no retry needed (highly reliable over DuPont) */
static void test_gray_capture(uint8_t round) {
  Vision_SetMode(VISION_MODE_GRAY);
  Vision_CaptureStart();
  uint8_t rdy = Vision_IsFrameReady();
  uint32_t sz = Vision_GetFrameSize();
  dbg_printf("  R%u GRAY: rdy=%u sz=%lu\r\n", round, rdy, (unsigned long)sz);
  chk((rdy == 1), "GRAY capture OK");
  chk((sz == GRAY_FRAME_SIZE), "GRAY size == 19200");
}

void Test_Mode_Switch_Run(void) {
  dbg_printf("======== TDD: Mode Switch ========\r\n");

  int8_t ret = Vision_Init();
  chk((ret == 0), "Vision_Init OK");
  if (ret != 0) {
    dbg_printf("[ABORT] Init failed\r\n");
    return;
  }

  for (uint8_t r = 0; r < SWITCH_ROUNDS; r++) {
    dbg_printf("-- Round %u/%u --\r\n", r + 1, SWITCH_ROUNDS);
    test_jpeg_capture(r + 1);
    test_gray_capture(r + 1);
  }

  dbg_printf("======== %u PASS, %u FAIL ========\r\n", s_pass, s_fail);
  dbg_printf(s_fail == 0 ? "[RESULT] ALL PASS\r\n"
                         : "[RESULT] HAS FAILURES\r\n");
}
