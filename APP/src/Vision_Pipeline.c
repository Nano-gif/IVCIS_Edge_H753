/**
 * @file    Vision_Pipeline.c
 * @brief   OV5640 JPEG/YUV422 dual-buffer capture management.
 */

#include "Vision_Pipeline.h"
#include "FreeRTOS.h"
#include "app_config.h"
#include "cmsis_os.h"
#include "debug_config.h"
#include "ov5640.h"
#include "task.h"
#include "vision_capture.h"

extern DCMI_HandleTypeDef hdcmi;

D2_SRAM_SECTION IVCIS_ALIGN_32 static uint8_t
    s_jpeg_buf[FRAME_BUF_COUNT][JPEG_RX_BUF_SIZE];

static uint8_t s_gray_buf[GRAY_FRAME_SIZE];
static volatile BufState_t s_buf_state[FRAME_BUF_COUNT] = {BUF_FREE, BUF_FREE};
static uint8_t s_active_idx = 0;
static VisionMode_t s_mode = VISION_MODE_JPEG;
static uint8_t *s_frame_ptr = NULL;
static uint32_t s_frame_len = 0;

static void extract_gray(void) {
  for (uint32_t i = 0; i < GRAY_FRAME_SIZE; i++) {
    s_gray_buf[i] = s_jpeg_buf[s_active_idx][i * 2];
  }
  s_frame_ptr = s_gray_buf;
  s_frame_len = GRAY_FRAME_SIZE;
}

static uint8_t scan_jpeg(void) {
  uint8_t *buf = s_jpeg_buf[s_active_idx];
  uint32_t soi = UINT32_MAX;

  for (uint32_t i = 0; i < (JPEG_RX_BUF_SIZE - 1); i++) {
    if ((buf[i] == 0xFF) && (buf[i + 1] == 0xD8)) {
      soi = i;
      break;
    }
  }

  if (soi == UINT32_MAX) {
    return 0;
  }

  for (uint32_t i = soi + 2; i < (JPEG_RX_BUF_SIZE - 1); i++) {
    if ((buf[i] == 0xFF) && (buf[i + 1] == 0xD9)) {
      s_frame_ptr = &buf[soi];
      s_frame_len = i - soi + 2;
      return 1;
    }
  }

  return 0;
}

uint8_t Vision_GetActiveIdx(void) {
  taskENTER_CRITICAL();
  uint8_t idx = s_active_idx;
  taskEXIT_CRITICAL();
  return idx;
}

void Vision_SetBufState(uint8_t idx, BufState_t state) {
  if (idx >= FRAME_BUF_COUNT) {
    return;
  }

  taskENTER_CRITICAL();
  s_buf_state[idx] = state;
  taskEXIT_CRITICAL();
}

BufState_t Vision_GetBufState(uint8_t idx) {
  if (idx >= FRAME_BUF_COUNT) {
    return BUF_FREE;
  }

  taskENTER_CRITICAL();
  BufState_t state = s_buf_state[idx];
  taskEXIT_CRITICAL();
  return state;
}

uint8_t Vision_SwitchBuffer(void) {
  taskENTER_CRITICAL();
  uint8_t next = (s_active_idx + 1) % FRAME_BUF_COUNT;
  if (s_buf_state[next] != BUF_FREE) {
    taskEXIT_CRITICAL();
    return 0;
  }
  s_active_idx = next;
  taskEXIT_CRITICAL();
  return 1;
}

int8_t Vision_Init(void) {
  if (atk_mc5640_init() != ATK_MC5640_EOK) {
    DBG_VISION("OV5640 init FAIL");
    return -1;
  }

  atk_mc5640_set_test_pattern(ATK_MC5640_TEST_PATTERN_OFF);
  Vision_SetMode(VISION_MODE_GRAY);
  if (!Vision_CaptureOne(s_jpeg_buf[0], GRAY_FRAME_SIZE * 2 / 4U)) {
    DBG_VISION("Warmup TIMEOUT");
    return -1;
  }

  DBG_VISION("Init OK (dual-buf)");
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
  if (mode != VISION_MODE_JPEG) {
    uint32_t wl = GRAY_FRAME_SIZE * 2 / 4U;
    for (uint8_t d = 0; d < 2; d++) {
      Vision_CaptureOne(s_jpeg_buf[s_active_idx], wl);
    }
  }

  DBG_VISION("Mode %d ready", (int32_t)mode);
}

void Vision_CaptureStart(void) {
  s_frame_ptr = NULL;
  s_frame_len = 0;

  uint8_t *buf = s_jpeg_buf[s_active_idx];
  if (s_mode == VISION_MODE_JPEG) {
    if (Vision_CaptureOne(buf, JPEG_RX_BUF_SIZE / 4U)) {
      if (scan_jpeg()) {
        DBG_VISION("JPEG %lu B [%d]", s_frame_len, s_active_idx);
      }
    }
  } else {
    if (Vision_CaptureOne(buf, GRAY_FRAME_SIZE * 2 / 4U)) {
      extract_gray();
      DBG_VISION("Gray OK");
    }
  }
}

uint8_t Vision_IsFrameReady(void) { return (s_frame_ptr != NULL) ? 1U : 0U; }
uint8_t *Vision_GetFrameBuffer(void) { return s_frame_ptr; }
uint32_t Vision_GetFrameSize(void) { return s_frame_len; }
