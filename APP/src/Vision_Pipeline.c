/**
 * @file    Vision_Pipeline.c
 * @brief   OV5640 JPEG/Gray capture pipeline + SOI/EOI scan
 * @note    Uses osDelay for RTOS-friendly waiting
 */

#include "Vision_Pipeline.h"
#include "app_config.h"
#include "cmsis_os.h"
#include "debug_config.h"
#include "ov5640.h"

extern DCMI_HandleTypeDef hdcmi;
extern volatile uint8_t g_ov5640_frame_cplt;

/* JPEG/YUV raw buffer, D2 SRAM (Non-cacheable via MPU) */
D2_SRAM_SECTION IVCIS_ALIGN_32 static uint8_t s_raw_buf[JPEG_RX_BUF_SIZE];

/* Gray frame buffer (160x120, static) */
static uint8_t s_gray_buf[GRAY_FRAME_SIZE];

static VisionMode_t s_mode = VISION_MODE_JPEG;
static uint8_t *s_frame_ptr = NULL;
static uint32_t s_frame_len = 0;

/* Max wait for frame completion (ms) */
#define FRAME_TIMEOUT_MS 2000

/**
 * @brief  Register-level force-stop DCMI+DMA
 */
static void force_stop(void) {
  hdcmi.Instance->CR &= ~(DCMI_CR_CAPTURE | DCMI_CR_ENABLE);
  ((DMA_Stream_TypeDef *)hdcmi.DMA_Handle->Instance)->CR &= ~DMA_SxCR_EN;
  hdcmi.Instance->ICR = 0x1FU;
  hdcmi.State = HAL_DCMI_STATE_READY;
  hdcmi.DMA_Handle->State = HAL_DMA_STATE_READY;
  hdcmi.Lock = HAL_UNLOCKED;
  hdcmi.DMA_Handle->Lock = HAL_UNLOCKED;
}

/**
 * @brief  Extract Y (Luminance) from YUV422
 */
static void extract_gray(void) {
  for (uint32_t i = 0; i < GRAY_FRAME_SIZE; i++) {
    s_gray_buf[i] = s_raw_buf[i * 2];
  }
  s_frame_ptr = s_gray_buf;
  s_frame_len = GRAY_FRAME_SIZE;
}

/**
 * @brief  Scan JPEG markers: SOI(FFD8) and EOI(FFD9)
 * @retval 1=complete frame found, 0=not found
 */
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

/**
 * @brief  Single-frame capture with RTOS-friendly wait
 * @param  word_len DMA transfer length (32-bit words)
 * @retval 1=OK, 0=timeout
 */
static uint8_t capture_one(uint32_t word_len) {
  force_stop();

  g_ov5640_frame_cplt = 0;
  HAL_DCMI_Start_DMA(&hdcmi, DCMI_MODE_SNAPSHOT, (uint32_t)s_raw_buf, word_len);

  /* RTOS-friendly polling: yield CPU between checks */
  uint32_t t0 = HAL_GetTick();
  while (g_ov5640_frame_cplt == 0) {
    if ((HAL_GetTick() - t0) > FRAME_TIMEOUT_MS) {
      DBG_VISION("Capture timeout!");
      force_stop();
      return 0;
    }
    osDelay(1);
  }

  HAL_DCMI_Stop(&hdcmi);
  return 1;
}

/* ===== Public API ===== */

int8_t Vision_Init(void) {
  if (atk_mc5640_init() != ATK_MC5640_EOK) {
    DBG_VISION("OV5640 init FAILED");
    return -1;
  }
  Vision_SetMode(VISION_MODE_JPEG);
  atk_mc5640_set_test_pattern(ATK_MC5640_TEST_PATTERN_OFF);

  /* Warm-up capture to flush sensor FIFO */
  capture_one(JPEG_RX_BUF_SIZE / 4U);
  DBG_VISION("Init OK");
  return 0;
}

void Vision_SetMode(VisionMode_t mode) {
  s_mode = mode;
  if (mode == VISION_MODE_JPEG) {
    atk_mc5640_set_output_format(ATK_MC5640_OUTPUT_FORMAT_JPEG);
    atk_mc5640_set_output_size(CAM_JPEG_WIDTH, CAM_JPEG_HEIGHT);
  } else {
    /* RGB565 base config + YUV422 overlay */
    atk_mc5640_set_output_format(ATK_MC5640_OUTPUT_FORMAT_RGB565);
    atk_mc5640_set_output_format(ATK_MC5640_OUTPUT_FORMAT_YUV422);
    atk_mc5640_set_output_size(CAM_GRAY_WIDTH, CAM_GRAY_HEIGHT);
  }
  /* RTOS-friendly stabilization wait */
  osDelay(300);
}

void Vision_CaptureStart(void) {
  s_frame_ptr = NULL;
  s_frame_len = 0;

  if (s_mode == VISION_MODE_JPEG) {
    if (capture_one(JPEG_RX_BUF_SIZE / 4U)) {
      if (scan_jpeg()) {
        DBG_VISION("JPEG %lu B", s_frame_len);
      }
    }
  } else {
    /* 160*120*2 = 38400 bytes = 9600 words */
    if (capture_one(9600)) {
      extract_gray();
      DBG_VISION("Gray frame ready");
    }
  }
}

uint8_t Vision_IsFrameReady(void) { return (s_frame_ptr != NULL) ? 1U : 0U; }

uint8_t *Vision_GetFrameBuffer(void) { return s_frame_ptr; }

uint32_t Vision_GetFrameSize(void) { return s_frame_len; }
