/**
 * @file    Vision_Pipeline.c
 * @brief   OV5640 JPEG/YUV422 模式管理 + 帧解析
 */

#include "Vision_Pipeline.h"
#include "app_config.h"
#include "cmsis_os.h"
#include "debug_config.h"
#include "ov5640.h"
#include "vision_capture.h"

extern DCMI_HandleTypeDef hdcmi;

/* JPEG/YUV 原始缓冲区, D2 SRAM (MPU Non-cacheable) */
D2_SRAM_SECTION IVCIS_ALIGN_32 static uint8_t s_raw_buf[JPEG_RX_BUF_SIZE];

static uint8_t s_gray_buf[GRAY_FRAME_SIZE];
static VisionMode_t s_mode = VISION_MODE_JPEG;
static uint8_t *s_frame_ptr = NULL;
static uint32_t s_frame_len = 0;

static void extract_gray(void) {
  for (uint32_t i = 0; i < GRAY_FRAME_SIZE; i++) {
    s_gray_buf[i] = s_raw_buf[i * 2];
  }
  s_frame_ptr = s_gray_buf;
  s_frame_len = GRAY_FRAME_SIZE;
}

static uint8_t scan_jpeg(void) {
  uint32_t soi = UINT32_MAX;
  for (uint32_t i = 0; i < (JPEG_RX_BUF_SIZE - 1); i++) {
    if ((s_raw_buf[i] == 0xFF) && (s_raw_buf[i + 1] == 0xD8)) {
      soi = i;
      break;
    }
  }
  if (soi == UINT32_MAX) {
    return 0;
  }
  for (uint32_t i = soi + 2; i < (JPEG_RX_BUF_SIZE - 1); i++) {
    if ((s_raw_buf[i] == 0xFF) && (s_raw_buf[i + 1] == 0xD9)) {
      s_frame_ptr = &s_raw_buf[soi];
      s_frame_len = i - soi + 2;
      return 1;
    }
  }
  return 0;
}

/* ===== 公共 API ===== */

int8_t Vision_Init(void) {
  if (atk_mc5640_init() != ATK_MC5640_EOK) {
    DBG_VISION("OV5640 init FAIL");
    return -1;
  }
  atk_mc5640_set_test_pattern(ATK_MC5640_TEST_PATTERN_OFF);
  Vision_SetMode(VISION_MODE_GRAY);
  if (!Vision_CaptureOne(s_raw_buf, GRAY_FRAME_SIZE * 2 / 4U)) {
    DBG_VISION("Warmup TIMEOUT");
    return -1;
  }
  DBG_VISION("Init OK");
  return 0;
}

void Vision_SetMode(VisionMode_t mode) {
  s_mode = mode;
  Vision_ForceStop();

  if (mode == VISION_MODE_JPEG) {
    hdcmi.Init.JPEGMode = DCMI_JPEG_ENABLE;
    HAL_DCMI_Init(&hdcmi);
    atk_mc5640_set_output_format(ATK_MC5640_OUTPUT_FORMAT_JPEG);
    atk_mc5640_set_output_size(CAM_JPEG_WIDTH, CAM_JPEG_HEIGHT);
  } else {
    hdcmi.Init.JPEGMode = DCMI_JPEG_DISABLE;
    HAL_DCMI_Init(&hdcmi);
    atk_mc5640_set_output_format(ATK_MC5640_OUTPUT_FORMAT_RGB565);
    atk_mc5640_set_output_format(ATK_MC5640_OUTPUT_FORMAT_YUV422);
    atk_mc5640_set_output_size(CAM_GRAY_WIDTH, CAM_GRAY_HEIGHT);
  }

  osDelay(300);

  /* GRAY: 丢弃前 2 帧脏数据 (ISP 等待)
   * JPEG: 跳过 — scan_jpeg() 已包含完整性校验 */
  if (mode != VISION_MODE_JPEG) {
    uint32_t wl = GRAY_FRAME_SIZE * 2 / 4U;
    for (uint8_t d = 0; d < 2; d++) {
      Vision_CaptureOne(s_raw_buf, wl);
    }
  }
  DBG_VISION("Mode %d ready", (int32_t)mode);
}

void Vision_CaptureStart(void) {
  s_frame_ptr = NULL;
  s_frame_len = 0;
  if (s_mode == VISION_MODE_JPEG) {
    if (Vision_CaptureOne(s_raw_buf, JPEG_RX_BUF_SIZE / 4U)) {
      if (scan_jpeg()) {
        DBG_VISION("JPEG %lu B", s_frame_len);
      }
    }
  } else {
    if (Vision_CaptureOne(s_raw_buf, GRAY_FRAME_SIZE * 2 / 4U)) {
      extract_gray();
      DBG_VISION("Gray OK");
    }
  }
}

uint8_t Vision_IsFrameReady(void) { return (s_frame_ptr != NULL) ? 1U : 0U; }
uint8_t *Vision_GetFrameBuffer(void) { return s_frame_ptr; }
uint32_t Vision_GetFrameSize(void) { return s_frame_len; }
