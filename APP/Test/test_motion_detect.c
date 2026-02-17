/**
 * @file    test_motion_detect.c
 * @brief   TDD: Motion_Detect isolated test with synthetic frames
 *
 * Uses memset/memcpy to build fake grayscale frames.
 * No camera or DCMI dependency — pure logic validation.
 */

#include "Motion_Detect.h"
#include "app_config.h"
#include "debug_config.h"
#include <string.h>

static uint8_t s_pass = 0;
static uint8_t s_fail = 0;

static uint8_t s_frame[GRAY_FRAME_SIZE];

static void chk(uint8_t cond, const char *msg) {
  if (cond) {
    s_pass++;
  } else {
    s_fail++;
  }
  dbg_printf("[%s] %s\r\n", cond ? "PASS" : "FAIL", msg);
}

void Test_Motion_Detect_Run(void) {
  dbg_printf("======== TDD: Motion_Detect ========\r\n");

  /* T1: NULL safety */
  Motion_Init(160, 120);
  chk(Motion_Detect(NULL) == false, "T1 NULL -> false");

  /* T2: First frame stores only, returns false */
  memset(s_frame, 0x80, GRAY_FRAME_SIZE);
  chk(Motion_Detect(s_frame) == false, "T2 first frame -> false");

  /* T3: Identical frame -> no motion */
  chk(Motion_Detect(s_frame) == false, "T3 same frame -> false");
  chk(Motion_GetDiff() == 0, "T3 diff == 0");

  /* T4: Tiny change (+5 per pixel) -> below threshold */
  memset(s_frame, 0x85, GRAY_FRAME_SIZE);
  chk(Motion_Detect(s_frame) == false, "T4 tiny diff -> false");
  dbg_printf("  T4 avg_diff=%lu\r\n", (unsigned long)Motion_GetDiff());

  /* T5: Large change (+127 per pixel) -> motion detected */
  memset(s_frame, 0x85 + 127, GRAY_FRAME_SIZE);
  chk(Motion_Detect(s_frame) == true, "T5 large diff -> true");
  dbg_printf("  T5 avg_diff=%lu\r\n", (unsigned long)Motion_GetDiff());

  /* T6: Partial change — 1000 pixels shift by 50, rest same */
  Motion_Init(160, 120);
  memset(s_frame, 0x40, GRAY_FRAME_SIZE);
  Motion_Detect(s_frame); /* store as prev */
  memset(s_frame, 0x40, GRAY_FRAME_SIZE);
  memset(s_frame, 0x40 + 50, 1000); /* first 1000 pixels changed */
  bool result = Motion_Detect(s_frame);
  dbg_printf("  T6 diff=%lu partial=%s\r\n", (unsigned long)Motion_GetDiff(),
             result ? "true" : "false");
  chk(result == true, "T6 partial 1000px -> true");

  /* T7: Init resets state — next detect is first-frame again */
  Motion_Init(160, 120);
  memset(s_frame, 0xAA, GRAY_FRAME_SIZE);
  chk(Motion_Detect(s_frame) == false, "T7 after Init -> false (1st)");

  dbg_printf("======== %u PASS, %u FAIL ========\r\n", s_pass, s_fail);
  dbg_printf(s_fail == 0 ? "[RESULT] ALL PASS\r\n"
                         : "[RESULT] HAS FAILURES\r\n");
}
