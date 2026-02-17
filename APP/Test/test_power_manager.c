/**
 * @file    test_power_manager.c
 * @brief   TDD: Power_Manager state machine isolated test
 *
 * Tests state transitions with REAL HAL_GetTick timing.
 * Uses shortened thresholds (2s/5s) to keep test under 15s.
 * Original thresholds: 30s / 300s (too long for TDD).
 *
 * NOTE: This test REQUIRES app_config.h thresholds to be
 * temporarily set to:
 *   PWR_FULL_TO_LIGHT_SEC  = 2
 *   PWR_LIGHT_TO_DETECT_SEC = 5
 */

#include "Power_Manager.h"
#include "app_config.h"
#include "debug_config.h"
#include "shared_types.h"
#include "stm32h7xx_hal.h"

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

/* T1-T2: Init + GetDelay */
static void test_init(void) {
  PowerMgr_Init();
  chk(PowerMgr_GetMode() == PWR_FULL, "T1 init -> FULL");
  chk(PowerMgr_GetDelay() == PWR_FULL_DELAY_MS, "T2 delay == 100ms");
}

/* T3: JPEG size-change detection */
static void test_jpeg_size_detect(void) {
  PowerMgr_Init();
  PowerMgr_Tick(10000, false); /* baseline */
  PowerMgr_Tick(10000, false); /* same size -> no motion */
  chk(PowerMgr_GetMode() == PWR_FULL, "T3a same JPEG -> FULL");

  /* 20% change (>15% threshold) */
  PowerMgr_Tick(12100, false);
  chk(PowerMgr_GetMode() == PWR_FULL, "T3b 20%% change -> stay FULL");
}

/* T4: FULL -> LIGHT timeout degradation */
static void test_full_to_light(void) {
  PowerMgr_Init();
  PowerMgr_Tick(10000, false); /* baseline */
  dbg_printf("  Waiting %us for FULL->LIGHT...\r\n", PWR_FULL_TO_LIGHT_SEC);
  HAL_Delay(PWR_FULL_TO_LIGHT_SEC * 1000 + 500);
  PowerMgr_Tick(10000, false); /* same size, time exceeded */
  chk(PowerMgr_GetMode() == PWR_LIGHT, "T4 timeout -> LIGHT");
  chk(PowerMgr_GetDelay() == PWR_LIGHT_DELAY_MS, "T4 delay == 500ms");
}

/* T5: LIGHT -> DETECT timeout degradation */
static void test_light_to_detect(void) {
  dbg_printf("  Waiting %us for LIGHT->DETECT...\r\n", PWR_LIGHT_TO_DETECT_SEC);
  HAL_Delay(PWR_LIGHT_TO_DETECT_SEC * 1000 + 500);
  PowerMgr_Tick(10000, false);
  chk(PowerMgr_GetMode() == PWR_DETECT, "T5 timeout -> DETECT");
  chk(PowerMgr_GetDelay() == PWR_DETECT_DELAY_MS, "T5 delay == 1000ms");
}

/* T6: DETECT -> FULL on gray motion */
static void test_motion_wakeup(void) {
  PowerMgr_Tick(0, true); /* gray motion detected */
  chk(PowerMgr_GetMode() == PWR_FULL, "T6 motion -> FULL");
}

void Test_Power_Manager_Run(void) {
  dbg_printf("======== TDD: Power_Manager ========\r\n");
  dbg_printf("[NOTE] Requires short thresholds: %us / %us\r\n",
             PWR_FULL_TO_LIGHT_SEC, PWR_LIGHT_TO_DETECT_SEC);

  test_init();
  test_jpeg_size_detect();
  test_full_to_light();
  test_light_to_detect();
  test_motion_wakeup();

  dbg_printf("======== %u PASS, %u FAIL ========\r\n", s_pass, s_fail);
  dbg_printf(s_fail == 0 ? "[RESULT] ALL PASS\r\n"
                         : "[RESULT] HAS FAILURES\r\n");
}
