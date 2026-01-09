#ifndef APP_CONFIG_H
#define APP_CONFIG_H

#include "stm32h7xx_hal.h"

/* 图像参数 */
#define CAM_RES_WIDTH       800
#define CAM_RES_HEIGHT      480
#define JPEG_STRIP_LINES    16
#define STRIP_BUFFER_SIZE   (CAM_RES_WIDTH * JPEG_STRIP_LINES * 2)
#define JPEG_OUT_BUFFER_SIZE (80 * 1024)

/* 网络参数 */
#define DEST_IP_ADDR0        192
#define DEST_IP_ADDR1        168
#define DEST_IP_ADDR2        1
#define DEST_IP_ADDR3        255  /* 临时改为广播地址测试 */
#define LOCAL_IP_ADDR0       192
#define LOCAL_IP_ADDR1       168
#define LOCAL_IP_ADDR2       1
#define LOCAL_IP_ADDR3       10
#define UDP_REMOTE_PORT      8080
#define UDP_LOCAL_PORT       8000

/* 内存段与对齐宏 (已更名以避免与 HAL 库冲突) */
#define IVCIS_ALIGN_32       __attribute__((aligned(32)))
#define D2_SRAM_SECTION      __attribute__((section(".RamDataSection")))
#define D1_AXI_SECTION       __attribute__((section(".RamDataSectionAXI")))

#endif
