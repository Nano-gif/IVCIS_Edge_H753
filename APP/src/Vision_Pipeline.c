#include "Vision_Pipeline.h"
#include "app_config.h"
#include "shared_types.h"
#include "Net_Client.h"
#include "main.h"
#include "ov5640.h"
#include "sccb.h"
#include <string.h>
#include <stdio.h>

/* ========================================== */
/* 1. 变量定义 (严格按照内存域与对齐规范)        */
/* ========================================== */
extern DCMI_HandleTypeDef hdcmi;
extern JPEG_HandleTypeDef hjpeg;

uint32_t half_transfer_count = 0;
uint32_t full_transfer_count = 0;

static uint16_t jpeg_strips_fed = 0;   // 已喂入 JPEG 核心的条带数
static uint32_t jpeg_total_out_size = 0; // 当前帧压缩后的总大小

D2_SRAM_SECTION IVCIS_ALIGN_32 uint8_t DCMI_Strip_Buf[2][STRIP_BUFFER_SIZE];
D1_AXI_SECTION IVCIS_ALIGN_32 uint8_t JPEG_Out_Buf[JPEG_OUT_BUFFER_SIZE];

/* ========================================== */
/* 2. JPEG 异步回调逻辑 (实现全帧流式压缩)      */
/* ========================================== */

/**
 * @brief  JPEG 输入回调：核心请求下一个条带
 */
void HAL_JPEG_GetDataCallback(JPEG_HandleTypeDef *hjpeg, uint32_t NbEncodedData) {
    // 800x480 共需 30 个条带 (480 / 16 = 30)
    if (jpeg_strips_fed < 30) {
        uint8_t *p_next_strip = DCMI_Strip_Buf[jpeg_strips_fed % 2];
        // 确保硬件看到的是内存中最新的像素
        SCB_CleanDCache_by_Addr((uint32_t*)p_next_strip, STRIP_BUFFER_SIZE);
        HAL_JPEG_ConfigInputBuffer(hjpeg, p_next_strip, STRIP_BUFFER_SIZE);
        jpeg_strips_fed++;
    }
}

/**
 * @brief  JPEG 输出回调：核心产生了一段压缩数据
 */
void HAL_JPEG_DataReadyCallback(JPEG_HandleTypeDef *hjpeg, uint8_t *pDataOut, uint32_t OutDataLength) {
     jpeg_total_out_size += OutDataLength;
}

/**
 * @brief  JPEG 完成回调：设置标志位通知任务发送
 * @note   不要在中断中直接调用 LwIP 发送函数，会导致 ETH 锁死
 */
volatile uint8_t jpeg_encode_complete = 0; // 编码完成标志
uint32_t last_jpeg_actual_size = 0;

void HAL_JPEG_EncodeCpltCallback(JPEG_HandleTypeDef *hjpeg) {
    last_jpeg_actual_size = jpeg_total_out_size;
    jpeg_encode_complete = 1; // 仅设置标志，由任务读取后发送
    printf("[JPEG] Frame %ld encoded, size=%ld bytes\r\n", full_transfer_count, jpeg_total_out_size);
}

/* ========================================== */
/* 3. 中断回调逻辑 (DCMI 采集节拍)             */
/* ========================================== */

void Vision_DMA_HalfTransfer_Callback(DMA_HandleTypeDef *hdma) {
    SCB_InvalidateDCache_by_Addr((uint32_t*)DCMI_Strip_Buf[0], STRIP_BUFFER_SIZE);
    half_transfer_count++;
    HAL_GPIO_TogglePin(LD1_GPIO_Port, LD1_Pin);
}

void Vision_DMA_FullTransfer_Callback(DMA_HandleTypeDef *hdma) {
    SCB_InvalidateDCache_by_Addr((uint32_t*)DCMI_Strip_Buf[1], STRIP_BUFFER_SIZE);
    full_transfer_count++;
    HAL_GPIO_TogglePin(LD3_GPIO_Port, LD3_Pin);

    // 每 50 帧执行一次全帧压缩上报 (控制带宽占用)
    if (full_transfer_count % 50 == 0) {
        JPEG_ConfTypeDef Conf;
        jpeg_strips_fed = 1; // 第一次调用 HAL_JPEG_Encode_DMA 会自动消耗第0个条带
        jpeg_total_out_size = 0;

        Conf.ColorSpace = JPEG_YCBCR_COLORSPACE;
        Conf.ChromaSubsampling = JPEG_420_SUBSAMPLING;
        Conf.ImageHeight = 480;
        Conf.ImageWidth = 800;
        Conf.ImageQuality = 75;
        HAL_JPEG_ConfigEncoding(&hjpeg, &Conf);

        // 启动异步压缩接力
        HAL_JPEG_Encode_DMA(&hjpeg, DCMI_Strip_Buf[0], STRIP_BUFFER_SIZE,
                            JPEG_Out_Buf, JPEG_OUT_BUFFER_SIZE);
    }
}

/* ========================================== */
/* 4. 初始化逻辑                              */
/* ========================================== */

int8_t Vision_Init(void) {
    if (atk_mc5640_init() != 0) return -1;

    // 物理层补丁：增强驱动电流 (0x302D) 并调优 PCLK
    SCCB_Start(); SCCB_WriteByte(0x78); SCCB_WriteByte(0x30); SCCB_WriteByte(0x2D); SCCB_WriteByte(0x3F); SCCB_Stop();
    SCCB_Start(); SCCB_WriteByte(0x78); SCCB_WriteByte(0x30); SCCB_WriteByte(0x36); SCCB_WriteByte(0x30); SCCB_Stop();
    SCCB_Start(); SCCB_WriteByte(0x78); SCCB_WriteByte(0x38); SCCB_WriteByte(0x24); SCCB_WriteByte(0x12); SCCB_Stop();

    atk_mc5640_set_output_format(ATK_MC5640_OUTPUT_FORMAT_RGB565);
    atk_mc5640_set_output_size(800, 480);
    HAL_Delay(100);

    // DCMI 极性微调 (上升沿采样更稳)
    HAL_DCMI_DeInit(&hdcmi);
    hdcmi.Init.PCKPolarity = DCMI_PCKPOLARITY_RISING;
    hdcmi.Init.VSPolarity = DCMI_VSPOLARITY_HIGH;
    hdcmi.Init.HSPolarity = DCMI_HSPOLARITY_LOW;
    HAL_DCMI_Init(&hdcmi);

    uint32_t dma_len_words = (STRIP_BUFFER_SIZE * 2) / 4;
    HAL_DCMI_Start_DMA(&hdcmi, DCMI_MODE_CONTINUOUS, (uint32_t)DCMI_Strip_Buf, dma_len_words);

    // 注册回调
    __HAL_DMA_ENABLE_IT(hdcmi.DMA_Handle, DMA_IT_TC | DMA_IT_HT);
    hdcmi.DMA_Handle->XferHalfCpltCallback = Vision_DMA_HalfTransfer_Callback;
    hdcmi.DMA_Handle->XferCpltCallback     = Vision_DMA_FullTransfer_Callback;

    return 0;
}
