/**
 * @file    test_vision_init.c
 * @brief   TDD Test: Vision_Init & JPEG capture + XCAM streaming mode
 */

#include "Vision_Pipeline.h"
#include "app_config.h"
#include "debug_config.h"
#include "stm32h7xx_hal.h"

extern DCMI_HandleTypeDef hdcmi;

static uint8_t tests_passed = 0;
static uint8_t tests_failed = 0;

static void assert_true(uint8_t cond, const char *msg) {
  if (cond) {
    tests_passed++;
    dbg_printf("[PASS] %s\r\n", msg);
  } else {
    tests_failed++;
    dbg_printf("[FAIL] %s\r\n", msg);
  }
}

static void test_init_returns_zero(void) {
  int8_t ret = Vision_Init();
  assert_true((ret == 0), "Vision_Init() == 0");
}

static void test_frame_not_ready_before_capture(void) {
  assert_true((Vision_IsFrameReady() == 0), "No frame before capture");
}

static void test_jpeg_frame_capture(void) {
  Vision_CaptureStart();
  uint8_t ready = Vision_IsFrameReady();

  assert_true((ready == 1), "Frame ready after capture");

  if (ready == 1) {
    uint8_t *buf = Vision_GetFrameBuffer();
    uint32_t len = Vision_GetFrameSize();
    assert_true((buf != NULL), "Buffer not NULL");
    HAL_Delay(10);
    dbg_printf("[INFO] Size=%lu B, Hdr: %02X %02X %02X %02X\r\n",
               (unsigned long)len, buf[0], buf[1], buf[2], buf[3]);
    assert_true((len < JPEG_RX_BUF_SIZE), "Frame < buf (no overflow)");
    assert_true(((buf[0] == 0xFF) && (buf[1] == 0xD8)), "JPEG SOI");
  }
}

/* XCAM streaming: continuous capture + UART send */
static void xcam_streaming_loop(void) {
  dbg_printf("[XCAM] Streaming start\r\n");
  HAL_Delay(500);
  while (1) {
    Vision_CaptureStart();
    if (Vision_IsFrameReady()) {
      Vision_SendFrameUART();
    }
  }
}

void Test_Vision_Init_Run(void) {
  dbg_printf("======== TDD: Vision Pipeline ========\r\n");

  test_init_returns_zero();
  test_frame_not_ready_before_capture();
  test_jpeg_frame_capture();

  dbg_printf("======== %u PASS, %u FAIL ========\r\n", tests_passed,
             tests_failed);

  /* Always enter streaming for XCAM debug */
  HAL_Delay(500);
  xcam_streaming_loop();
}
