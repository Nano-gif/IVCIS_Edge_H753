#ifndef APP_CONFIG_H
#define APP_CONFIG_H

#include "stm32h7xx_hal.h"

/* ========== Camera Parameters ========== */
#define CAM_JPEG_WIDTH 640
#define CAM_JPEG_HEIGHT 480
#define CAM_GRAY_WIDTH 160
#define CAM_GRAY_HEIGHT 120

/* JPEG receive buffer: OV5640 640x480 JPEG can exceed 80KB at high quality */
#define JPEG_RX_BUF_SIZE (100U * 1024U) /* Per-buffer, x2 for ping-pong */
#define GRAY_FRAME_SIZE (CAM_GRAY_WIDTH * CAM_GRAY_HEIGHT) /* 19200 B */

/* ========== Network Parameters ========== */
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

/* ========== Memory Section & Alignment ========== */
#define IVCIS_ALIGN_32 __attribute__((aligned(32)))
#define D2_SRAM_SECTION __attribute__((section(".RamDataSection")))
#define D1_AXI_SECTION __attribute__((section(".RamDataSectionAXI")))

#endif /* APP_CONFIG_H */
