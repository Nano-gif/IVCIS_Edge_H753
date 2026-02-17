#ifndef APP_CONFIG_H
#define APP_CONFIG_H

#include "stm32h7xx_hal.h"

/* ========== 摄像头参数 ========== */
#define CAM_JPEG_WIDTH 640
#define CAM_JPEG_HEIGHT 480
#define CAM_GRAY_WIDTH 160
#define CAM_GRAY_HEIGHT 120

/* JPEG 接收缓冲区: OV5640 640x480 高画质可达 80KB+ */
#define JPEG_RX_BUF_SIZE (100U * 1024U)
/* 灰度帧大小: 160*120 = 19200 B */
#define GRAY_FRAME_SIZE (CAM_GRAY_WIDTH * CAM_GRAY_HEIGHT)

/* ========== 网络参数 ========== */
#define NET_LINK_TIMEOUT_MS 3000
#define NET_MAX_UDP_PAYLOAD 1400
#define DEST_IP_ADDR0 192
#define DEST_IP_ADDR1 168
#define DEST_IP_ADDR2 1
#define DEST_IP_ADDR3 100
#define LOCAL_IP_ADDR0 192
#define LOCAL_IP_ADDR1 168
#define LOCAL_IP_ADDR2 1
#define LOCAL_IP_ADDR3 10
#define UDP_REMOTE_PORT 8080
#define UDP_LOCAL_PORT 8000

/* ========== 功耗模式参数 ========== */
#define PWR_FULL_TO_LIGHT_SEC 30    /* FULL->LIGHT 无运动时间(秒) */
#define PWR_LIGHT_TO_DETECT_SEC 300 /* LIGHT->DETECT 无运动时间(秒) */
#define PWR_JPEG_SIZE_DIFF_PCT 15   /* JPEG 尺寸差阈值(百分比) */
#define PWR_FULL_DELAY_MS 100       /* FULL 帧间隔(ms) ~10fps */
#define PWR_LIGHT_DELAY_MS 500      /* LIGHT 帧间隔(ms) ~2fps */
#define PWR_DETECT_DELAY_MS 1000    /* DETECT 帧间隔(ms) ~1fps */

/* ========== 内存段 & 对齐 ========== */
#define IVCIS_ALIGN_32 __attribute__((aligned(32)))
#define D2_SRAM_SECTION __attribute__((section(".RamDataSection")))
#define D1_AXI_SECTION __attribute__((section(".RamDataSectionAXI")))

#endif /* APP_CONFIG_H */
