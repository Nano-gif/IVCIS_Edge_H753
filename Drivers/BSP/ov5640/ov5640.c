/**
 ****************************************************************************************************
 * @file        atk_mc5640.c
 * @author      正点原子团队(ALIENTEK) (移植适配: IVCIS Project)
 * @version     V1.1
 * @date        2023-10-27
 * @brief       ATK-MC5640模块驱动代码 (HAL库适配版)
 * @license     Copyright (c) 2020-2032, 广州市星翼电子科技有限公司
 ****************************************************************************************************
 * @attention
 *
 * 硬件平台: Nucleo-H753ZI + OV5640
 * 适配说明:
 * 1. 移除模拟I2C，改用 STM32 HAL 硬件 I2C (hi2c1)
 * 2. 移除底层 GPIO 初始化，改用 CubeMX 生成的 main.h 定义
 * 3. 适配 DCMI 与 DMA 接口
 *
 ****************************************************************************************************
 */

#include "ov5640.h"
#include "ov5640_cfg.h"
#include "main.h"
#include "stm32h7xx_hal.h"
#include "dcmi.h"          // 获取 hdcmi
#include "sccb.h"

/* OV5640 设备地址 (7-bit: 0x3C, 8-bit write: 0x78) */
#define ATK_MC5640_SCCB_ADDR 0x78

/* ATK-MC5640模块芯片ID */
#define ATK_MC5640_CHIP_ID  0x5640

/* ATK-MC5640模块固件下载超时时间，单位：毫秒（ms） */
#define ATK_MC5640_TIMEOUT  5000

/* ATK-MC5640寄存器块枚举 */
typedef enum
{
    ATK_MC5640_REG_BANK_DSP = 0x00, /* DSP寄存器块 */
    ATK_MC5640_REG_BANK_SENSOR,     /* Sensor寄存器块 */
} atk_mc5640_reg_bank_t;

/* ATK-MC5640模块数据结构体 */
static struct
{
    struct {
        uint16_t width;
        uint16_t height;
    } isp_input;
    struct {
        uint16_t width;
        uint16_t height;
    } pre_scaling;
    struct {
        uint16_t width;
        uint16_t height;
    } output;
} g_atk_mc5640_sta = {0};

/* 外部 DCMI 传输完成标志位 (需要在中断回调中置位) */
volatile uint8_t g_ov5640_frame_cplt = 0;

/**
 * @brief       ATK-MC5640模块写寄存器 (HAL库硬件I2C适配)
 * @param       reg: 寄存器地址
 *              dat: 待写入的值
 * @retval      无
 */
void atk_mc5640_write_reg(uint16_t reg, uint8_t dat)
{
    SCCB_Start();
    SCCB_WriteByte(ATK_MC5640_SCCB_ADDR); // 写设备地址
    SCCB_WriteByte(reg >> 8);             // 写寄存器高8位
    SCCB_WriteByte(reg & 0xFF);           // 写寄存器低8位
    SCCB_WriteByte(dat);                  // 写数据
    SCCB_Stop();
}

// 读寄存器
uint8_t atk_mc5640_read_reg(uint16_t reg)
{
    uint8_t dat;

    SCCB_Start();
    SCCB_WriteByte(ATK_MC5640_SCCB_ADDR);
    SCCB_WriteByte(reg >> 8);
    SCCB_WriteByte(reg & 0xFF);
    SCCB_Stop();

    SCCB_Start();
    SCCB_WriteByte(ATK_MC5640_SCCB_ADDR | 0x01); // 读设备地址
    dat = SCCB_ReadByte();
    SCCB_No_Ack(); // 发送 NACK
    SCCB_Stop();

    return dat;
}
/**
 * @brief       获取ATK-MC5640模块ISP输入窗口尺寸
 * @param       无
 * @retval      无
 */
static void atk_mc5640_get_isp_input_size(void)
{
    uint8_t reg3800;
    uint8_t reg3801;
    uint8_t reg3802;
    uint8_t reg3803;
    uint8_t reg3804;
    uint8_t reg3805;
    uint8_t reg3806;
    uint8_t reg3807;
    uint16_t x_addr_st;
    uint16_t y_addr_st;
    uint16_t x_addr_end;
    uint16_t y_addr_end;
    
    HAL_Delay(10);
    
    reg3800 = atk_mc5640_read_reg(0x3800);
    reg3801 = atk_mc5640_read_reg(0x3801);
    reg3802 = atk_mc5640_read_reg(0x3802);
    reg3803 = atk_mc5640_read_reg(0x3803);
    reg3804 = atk_mc5640_read_reg(0x3804);
    reg3805 = atk_mc5640_read_reg(0x3805);
    reg3806 = atk_mc5640_read_reg(0x3806);
    reg3807 = atk_mc5640_read_reg(0x3807);
    
    x_addr_st = (uint16_t)((reg3800 & 0x0F) << 8) | reg3801;
    y_addr_st = (uint16_t)((reg3802 & 0x07) << 8) | reg3803;
    x_addr_end = (uint16_t)((reg3804 & 0x0F) << 8) | reg3805;
    y_addr_end = (uint16_t)((reg3806 & 0x07) << 8) | reg3807;
    
    g_atk_mc5640_sta.isp_input.width = x_addr_end - x_addr_st;
    g_atk_mc5640_sta.isp_input.height = y_addr_end - y_addr_st;
}

/**
 * @brief       获取ATK-MC5640模块预缩放窗口尺寸
 * @param       无
 * @retval      无
 */
static void atk_mc5640_get_pre_scaling_size(void)
{
    uint8_t reg3810;
    uint8_t reg3811;
    uint8_t reg3812;
    uint8_t reg3813;
    uint16_t x_offset;
    uint16_t y_offset;
    
    HAL_Delay(10);
    
    reg3810 = atk_mc5640_read_reg(0x3810);
    reg3811 = atk_mc5640_read_reg(0x3811);
    reg3812 = atk_mc5640_read_reg(0x3812);
    reg3813 = atk_mc5640_read_reg(0x3813);
    
    x_offset = (uint16_t)((reg3810 & 0x0F) << 8) | reg3811;
    y_offset = (uint16_t)((reg3812 & 0x07) << 8) | reg3813;
    
    atk_mc5640_get_isp_input_size();
    
    g_atk_mc5640_sta.pre_scaling.width = g_atk_mc5640_sta.isp_input.width - (x_offset << 1);
    g_atk_mc5640_sta.pre_scaling.height = g_atk_mc5640_sta.isp_input.height - (y_offset << 1);
}

/**
 * @brief       获取ATK-MC5640模块输出图像尺寸
 * @param       无
 * @retval      无
 */
static void atk_mc5640_get_output_size(void)
{
    uint8_t reg3808;
    uint8_t reg3809;
    uint8_t reg380A;
    uint8_t reg380B;
    uint16_t x_output_size;
    uint16_t y_output_size;
    
    HAL_Delay(10);
    
    reg3808 = atk_mc5640_read_reg(0x3808);
    reg3809 = atk_mc5640_read_reg(0x3809);
    reg380A = atk_mc5640_read_reg(0x380A);
    reg380B = atk_mc5640_read_reg(0x380B);
    
    x_output_size = (uint16_t)((reg3808 & 0x0F) << 8) | reg3809;
    y_output_size = (uint16_t)((reg380A & 0x07) << 8) | reg380B;
    
    g_atk_mc5640_sta.output.width = x_output_size;
    g_atk_mc5640_sta.output.height = y_output_size;
}

/**
 * @brief       ATK-MC5640模块硬件初始化
 * @note        GPIO的时钟使能和模式配置已在 main.c 的 MX_GPIO_Init() 中完成
 *              这里只需要进行电平控制序列
 */
static void atk_mc5640_hw_init(void)
{
    HAL_GPIO_WritePin(CAMERA_PWDN_GPIO_Port, CAMERA_PWDN_Pin, GPIO_PIN_RESET);
    HAL_Delay(20);

    HAL_GPIO_WritePin(CAMERA_RST_GPIO_Port, CAMERA_RST_Pin, GPIO_PIN_RESET);
    HAL_Delay(20);
    HAL_GPIO_WritePin(CAMERA_RST_GPIO_Port, CAMERA_RST_Pin, GPIO_PIN_SET);
    HAL_Delay(20);
}

/**
 * @brief       ATK-MC5640模块退出掉电模式 (软件调用)
 * @param       无
 * @retval      无
 */
static void atk_mc5640_exit_power_down(void)
{
    HAL_GPIO_WritePin(CAMERA_RST_GPIO_Port, CAMERA_RST_Pin, GPIO_PIN_RESET);
    HAL_Delay(20);
    HAL_GPIO_WritePin(CAMERA_PWDN_GPIO_Port, CAMERA_PWDN_Pin, GPIO_PIN_RESET);
    HAL_Delay(5);
    HAL_GPIO_WritePin(CAMERA_RST_GPIO_Port, CAMERA_RST_Pin, GPIO_PIN_SET);
    HAL_Delay(20);
}

/**
 * @brief       ATK-MC5640模块硬件复位
 * @param       无
 * @retval      无
 */
static void atk_mc5640_hw_reset(void)
{
    HAL_GPIO_WritePin(CAMERA_RST_GPIO_Port, CAMERA_RST_Pin, GPIO_PIN_RESET);
    HAL_Delay(20);
    HAL_GPIO_WritePin(CAMERA_RST_GPIO_Port, CAMERA_RST_Pin, GPIO_PIN_SET);
    HAL_Delay(20);
}

/**
 * @brief       ATK-MC5640模块软件复位
 * @param       无
 * @retval      无
 */
static void atk_mc5640_sw_reset(void)
{
    uint8_t reg3103;
    
    reg3103 = atk_mc5640_read_reg(0x3103);
    reg3103 &= ~(0x01 << 1);
    atk_mc5640_write_reg(0x3103, reg3103);
    atk_mc5640_write_reg(0x3008, 0x82);
    HAL_Delay(10);
}

/**
 * @brief       获取ATK-MC5640模块芯片ID
 * @param       无
 * @retval      芯片ID
 */
static uint16_t atk_mc5640_get_chip_id(void)
{
    uint16_t chip_id;
    
    chip_id = atk_mc5640_read_reg(0x300A) << 8;
    chip_id |= atk_mc5640_read_reg(0x300B);
    
    return chip_id;
}

/**
 * @brief       初始化ATK-MC5640寄存器配置
 * @param       无
 * @retval      无
 */
static void atk_mc5640_init_reg(void)
{
    uint32_t cfg_index;
    
    for (cfg_index=0; cfg_index<sizeof(atk_mc5640_init_cfg)/sizeof(atk_mc5640_init_cfg[0]); cfg_index++)
    {
        atk_mc5640_write_reg(atk_mc5640_init_cfg[cfg_index].reg, atk_mc5640_init_cfg[cfg_index].dat);
    }
}

/**
 * @brief       初始化ATK-MC5640模块
 * @param       无
 * @retval      ATK_MC5640_EOK   : ATK-MC5640模块初始化成功
 *              ATK_MC5640_ERROR : 通讯出错，ATK-MC5640模块初始化失败
 */
uint8_t atk_mc5640_init(void)
{
    uint16_t chip_id;
    SCCB_Init();
    atk_mc5640_hw_init();
    chip_id = atk_mc5640_get_chip_id();
    
    if (chip_id != ATK_MC5640_CHIP_ID)
    {
        printf("[Error] ID Mismatch! Read: 0x%04X\r\n", chip_id);
        return ATK_MC5640_ERROR;
    }
    
    printf("[Info] OV5640 ID: 0x%04X\r\n", chip_id);
    atk_mc5640_init_reg();
    atk_mc5640_write_reg(0x3035, 0x41);
    atk_mc5640_write_reg(0x3036, 0x30);
    
    return ATK_MC5640_EOK;
}

/**
 * @brief       初始化ATK-MC5640模块自动对焦
 * @param       无
 * @retval      ATK_MC5640_EOK     : ATK-MC5640模块自动对焦初始化成功
 *              ATK_MC5640_ETIMEOUT: ATK-MC5640模块下载固件超时
 */
uint8_t atk_mc5640_auto_focus_init(void)
{
    uint32_t fw_index;
    uint16_t addr_index;
    uint8_t reg3029 = 0;
    uint16_t timeout = 0;
    
    atk_mc5640_write_reg(0x3000, 0x20);
    
    for (addr_index=ATK_MC5640_FW_DOWNLOAD_ADDR, fw_index=0; fw_index<sizeof(atk_mc5640_auto_focus_firmware);addr_index++, fw_index++)
    {
        atk_mc5640_write_reg(addr_index, atk_mc5640_auto_focus_firmware[fw_index]);
    }
    
    atk_mc5640_write_reg(0x3022, 0x00);
    atk_mc5640_write_reg(0x3023, 0x00);
    atk_mc5640_write_reg(0x3024, 0x00);
    atk_mc5640_write_reg(0x3025, 0x00);
    atk_mc5640_write_reg(0x3026, 0x00);
    atk_mc5640_write_reg(0x3027, 0x00);
    atk_mc5640_write_reg(0x3028, 0x00);
    atk_mc5640_write_reg(0x3029, 0x7F);
    atk_mc5640_write_reg(0x3000, 0x00);
    
    while ((reg3029 != 0x70) && (timeout < ATK_MC5640_TIMEOUT))
    {
        HAL_Delay(1);
        reg3029 = atk_mc5640_read_reg(0x3029);
        timeout++;
    }
    
    if (reg3029 != 0x70)
    {
        return ATK_MC5640_ETIMEOUT;
    }
    
    return ATK_MC5640_EOK;
}

/**
 * @brief       ATK-MC5640模块自动对焦一次
 * @param       无
 * @retval      ATK_MC5640_EOK     : ATK-MC5640模块自动对焦一次成功
 *              ATK_MC5640_ETIMEOUT: ATK-MC5640模块自动对焦一次超时
 */
uint8_t atk_mc5640_auto_focus_once(void)
{
    uint8_t reg3029 = 0;
    uint16_t timeout = 0;
    
    atk_mc5640_write_reg(0x3022, 0x03);
    
    while ((reg3029 != 0x10) && (timeout < ATK_MC5640_TIMEOUT))
    {
        HAL_Delay(1);
        reg3029 = atk_mc5640_read_reg(0x3029);
        timeout++;
    }
    
    if (reg3029 != 0x10)
    {
        return ATK_MC5640_ETIMEOUT;
    }
    
    return ATK_MC5640_EOK;
}

/**
 * @brief       ATK-MC5640模块持续自动对焦
 * @param       无
 * @retval      ATK_MC5640_EOK     : ATK-MC5640模块持续自动对焦成功
 *              ATK_MC5640_ETIMEOUT: ATK-MC5640模块持续自动对焦超时
 */
uint8_t atk_mc5640_auto_focus_continuance(void)
{
    uint8_t reg3023 = ~0;
    uint16_t timeout = 0;
    
    atk_mc5640_write_reg(0x3023, 0x01);
    atk_mc5640_write_reg(0x3022, 0x08);
    
    while ((reg3023 != 0x00) && (timeout < ATK_MC5640_TIMEOUT))
    {
        HAL_Delay(1);
        reg3023 = atk_mc5640_read_reg(0x3023);
        timeout++;
    }
    
    if (reg3023 != 0x00)
    {
        return ATK_MC5640_ETIMEOUT;
    }
    
    reg3023 = ~0;
    timeout = 0;
    
    atk_mc5640_write_reg(0x3023, 0x01);
    atk_mc5640_write_reg(0x3022, 0x04);
    
    while ((reg3023 != 0x00) && (timeout < ATK_MC5640_TIMEOUT))
    {
        HAL_Delay(1);
        reg3023 = atk_mc5640_read_reg(0x3023);
        timeout++;
    }
    
    if (reg3023 != 0x00)
    {
        return ATK_MC5640_ETIMEOUT;
    }
    
    return ATK_MC5640_EOK;
}

/**
 * @brief       开启ATK-MC5640模块闪光灯
 * @param       无
 * @retval      无
 */
void atk_mc5640_led_on(void)
{
    atk_mc5640_write_reg(0x3016, 0x02);
    atk_mc5640_write_reg(0x301C, 0x02);
    atk_mc5640_write_reg(0x3019, 0x02);
}

/**
 * @brief       关闭ATK-MC5640模块闪光灯
 * @param       无
 * @retval      无
 */
void atk_mc5640_led_off(void)
{
    atk_mc5640_write_reg(0x3016, 0x02);
    atk_mc5640_write_reg(0x301C, 0x02);
    atk_mc5640_write_reg(0x3019, 0x00);
}

/**
 * @brief       设置ATK-MC5640模块灯光模式
 * @param       mode: 模式枚举
 * @retval      ATK_MC5640_EOK: 成功
 */
uint8_t atk_mc5640_set_light_mode(atk_mc5640_light_mode_t mode)
{
    switch (mode)
    {
        case ATK_MC5640_LIGHT_MODE_ADVANCED_AWB:
        {
            atk_mc5640_write_reg(0x3406, 0x00);
            atk_mc5640_write_reg(0x5192, 0x04);
            atk_mc5640_write_reg(0x5191, 0xF8);
            atk_mc5640_write_reg(0x5193, 0x70);
            atk_mc5640_write_reg(0x5194, 0xF0);
            atk_mc5640_write_reg(0x5195, 0xF0);
            atk_mc5640_write_reg(0x518D, 0x3D);
            atk_mc5640_write_reg(0x518F, 0x54);
            atk_mc5640_write_reg(0x518E, 0x3D);
            atk_mc5640_write_reg(0x5190, 0x54);
            atk_mc5640_write_reg(0x518B, 0xA8);
            atk_mc5640_write_reg(0x518C, 0xA8);
            atk_mc5640_write_reg(0x5187, 0x18);
            atk_mc5640_write_reg(0x5188, 0x18);
            atk_mc5640_write_reg(0x5189, 0x6E);
            atk_mc5640_write_reg(0x518A, 0x68);
            atk_mc5640_write_reg(0x5186, 0x1C);
            atk_mc5640_write_reg(0x5181, 0x50);
            atk_mc5640_write_reg(0x5184, 0x25);
            atk_mc5640_write_reg(0x5182, 0x11);
            atk_mc5640_write_reg(0x5183, 0x14);
            atk_mc5640_write_reg(0x5184, 0x25);
            atk_mc5640_write_reg(0x5185, 0x24);
            break;
        }
        case ATK_MC5640_LIGHT_MODE_SIMPLE_AWB:
        {
            atk_mc5640_write_reg(0x3406, 0x00);
            atk_mc5640_write_reg(0x5183, 0x94);
            atk_mc5640_write_reg(0x5191, 0xFF);
            atk_mc5640_write_reg(0x5192, 0x00);
            break;
        }
        case ATK_MC5640_LIGHT_MODE_MANUAL_DAY:
        {
            atk_mc5640_write_reg(0x3406, 0x01);
            atk_mc5640_write_reg(0x3400, 0x06);
            atk_mc5640_write_reg(0x3401, 0x1C);
            atk_mc5640_write_reg(0x3402, 0x04);
            atk_mc5640_write_reg(0x3403, 0x00);
            atk_mc5640_write_reg(0x3404, 0x04);
            atk_mc5640_write_reg(0x3405, 0xF3);
            break;
        }
        case ATK_MC5640_LIGHT_MODE_MANUAL_A:
        {
            atk_mc5640_write_reg(0x3406, 0x01);
            atk_mc5640_write_reg(0x3400, 0x04);
            atk_mc5640_write_reg(0x3401, 0x10);
            atk_mc5640_write_reg(0x3402, 0x04);
            atk_mc5640_write_reg(0x3403, 0x00);
            atk_mc5640_write_reg(0x3404, 0x08);
            atk_mc5640_write_reg(0x3405, 0xB6);
            break;
        }
        case ATK_MC5640_LIGHT_MODE_MANUAL_CWF:
        {
            atk_mc5640_write_reg(0x3406, 0x01);
            atk_mc5640_write_reg(0x3400, 0x05);
            atk_mc5640_write_reg(0x3401, 0x48);
            atk_mc5640_write_reg(0x3402, 0x04);
            atk_mc5640_write_reg(0x3403, 0x00);
            atk_mc5640_write_reg(0x3404, 0x07);
            atk_mc5640_write_reg(0x3405, 0xCF);
            break;
        }
        case ATK_MC5640_LIGHT_MODE_MANUAL_CLOUDY:
        {
            atk_mc5640_write_reg(0x3406, 0x01);
            atk_mc5640_write_reg(0x3400, 0x06);
            atk_mc5640_write_reg(0x3401, 0x48);
            atk_mc5640_write_reg(0x3402, 0x04);
            atk_mc5640_write_reg(0x3403, 0x00);
            atk_mc5640_write_reg(0x3404, 0x04);
            atk_mc5640_write_reg(0x3405, 0xD3);
            break;
        }
        default:
        {
            return ATK_MC5640_EINVAL;
        }
    }
    
    return ATK_MC5640_EOK;
}

/**
 * @brief       设置ATK-MC5640模块色彩饱和度
 * @param       saturation: 饱和度等级
 * @retval      ATK_MC5640_EOK: 成功
 */
uint8_t atk_mc5640_set_color_saturation(atk_mc5640_color_saturation_t saturation)
{
    switch (saturation)
    {
        case ATK_MC5640_COLOR_SATURATION_0:
        {
            atk_mc5640_write_reg(0x5001, 0xFF);
            atk_mc5640_write_reg(0x5583, 0x80);
            atk_mc5640_write_reg(0x5584, 0x80);
            atk_mc5640_write_reg(0x5580, 0x02);
            atk_mc5640_write_reg(0x5588, 0x41);
            break;
        }
        case ATK_MC5640_COLOR_SATURATION_1:
        {
            atk_mc5640_write_reg(0x5001, 0xFF);
            atk_mc5640_write_reg(0x5583, 0x70);
            atk_mc5640_write_reg(0x5584, 0x70);
            atk_mc5640_write_reg(0x5580, 0x02);
            atk_mc5640_write_reg(0x5588, 0x41);
            break;
        }
        case ATK_MC5640_COLOR_SATURATION_2:
        {
            atk_mc5640_write_reg(0x5001, 0xFF);
            atk_mc5640_write_reg(0x5583, 0x60);
            atk_mc5640_write_reg(0x5584, 0x60);
            atk_mc5640_write_reg(0x5580, 0x02);
            atk_mc5640_write_reg(0x5588, 0x41);
            break;
        }
        case ATK_MC5640_COLOR_SATURATION_3:
        {
            atk_mc5640_write_reg(0x5001, 0xFF);
            atk_mc5640_write_reg(0x5583, 0x50);
            atk_mc5640_write_reg(0x5584, 0x50);
            atk_mc5640_write_reg(0x5580, 0x02);
            atk_mc5640_write_reg(0x5588, 0x41);
            break;
        }
        case ATK_MC5640_COLOR_SATURATION_4:
        {
            atk_mc5640_write_reg(0x5001, 0xFF);
            atk_mc5640_write_reg(0x5583, 0x40);
            atk_mc5640_write_reg(0x5584, 0x40);
            atk_mc5640_write_reg(0x5580, 0x02);
            atk_mc5640_write_reg(0x5588, 0x41);
            break;
        }
        case ATK_MC5640_COLOR_SATURATION_5:
        {
            atk_mc5640_write_reg(0x5001, 0xFF);
            atk_mc5640_write_reg(0x5583, 0x30);
            atk_mc5640_write_reg(0x5584, 0x30);
            atk_mc5640_write_reg(0x5580, 0x02);
            atk_mc5640_write_reg(0x5588, 0x41);
            break;
        }
        case ATK_MC5640_COLOR_SATURATION_6:
        {
            atk_mc5640_write_reg(0x5001, 0xFF);
            atk_mc5640_write_reg(0x5583, 0x20);
            atk_mc5640_write_reg(0x5584, 0x20);
            atk_mc5640_write_reg(0x5580, 0x02);
            atk_mc5640_write_reg(0x5588, 0x41);
            break;
        }
        case ATK_MC5640_COLOR_SATURATION_7:
        {
            atk_mc5640_write_reg(0x5001, 0xFF);
            atk_mc5640_write_reg(0x5583, 0x10);
            atk_mc5640_write_reg(0x5584, 0x10);
            atk_mc5640_write_reg(0x5580, 0x02);
            atk_mc5640_write_reg(0x5588, 0x41);
            break;
        }
        case ATK_MC5640_COLOR_SATURATION_8:
        {
            atk_mc5640_write_reg(0x5001, 0xFF);
            atk_mc5640_write_reg(0x5583, 0x00);
            atk_mc5640_write_reg(0x5584, 0x00);
            atk_mc5640_write_reg(0x5580, 0x02);
            atk_mc5640_write_reg(0x5588, 0x41);
            break;
        }
        default:
        {
            return ATK_MC5640_EINVAL;
        }
    }
    
    return ATK_MC5640_EOK;
}

/**
 * @brief       设置ATK-MC5640模块亮度
 * @param       brightness: 亮度等级
 * @retval      ATK_MC5640_EOK: 成功
 */
uint8_t atk_mc5640_set_brightness(atk_mc5640_brightness_t brightness)
{
    switch (brightness)
    {
        case ATK_MC5640_BRIGHTNESS_0:
        {
            atk_mc5640_write_reg(0x5001, 0xFF);
            atk_mc5640_write_reg(0x5587, 0x40);
            atk_mc5640_write_reg(0x5580, 0x04);
            atk_mc5640_write_reg(0x5588, 0x01);
            break;
        }
        case ATK_MC5640_BRIGHTNESS_1:
        {
            atk_mc5640_write_reg(0x5001, 0xFF);
            atk_mc5640_write_reg(0x5587, 0x30);
            atk_mc5640_write_reg(0x5580, 0x04);
            atk_mc5640_write_reg(0x5588, 0x01);
            break;
        }
        case ATK_MC5640_BRIGHTNESS_2:
        {
            atk_mc5640_write_reg(0x5001, 0xFF);
            atk_mc5640_write_reg(0x5587, 0x20);
            atk_mc5640_write_reg(0x5580, 0x04);
            atk_mc5640_write_reg(0x5588, 0x01);
            break;
        }
        case ATK_MC5640_BRIGHTNESS_3:
        {
            atk_mc5640_write_reg(0x5001, 0xFF);
            atk_mc5640_write_reg(0x5587, 0x10);
            atk_mc5640_write_reg(0x5580, 0x04);
            atk_mc5640_write_reg(0x5588, 0x01);
            break;
        }
        case ATK_MC5640_BRIGHTNESS_4:
        {
            atk_mc5640_write_reg(0x5001, 0xFF);
            atk_mc5640_write_reg(0x5587, 0x00);
            atk_mc5640_write_reg(0x5580, 0x04);
            atk_mc5640_write_reg(0x5588, 0x01);
            break;
        }
        case ATK_MC5640_BRIGHTNESS_5:
        {
            atk_mc5640_write_reg(0x5001, 0xFF);
            atk_mc5640_write_reg(0x5587, 0x10);
            atk_mc5640_write_reg(0x5580, 0x04);
            atk_mc5640_write_reg(0x5588, 0x09);
            break;
        }
        case ATK_MC5640_BRIGHTNESS_6:
        {
            atk_mc5640_write_reg(0x5001, 0xFF);
            atk_mc5640_write_reg(0x5587, 0x20);
            atk_mc5640_write_reg(0x5580, 0x04);
            atk_mc5640_write_reg(0x5588, 0x09);
            break;
        }
        case ATK_MC5640_BRIGHTNESS_7:
        {
            atk_mc5640_write_reg(0x5001, 0xFF);
            atk_mc5640_write_reg(0x5587, 0x30);
            atk_mc5640_write_reg(0x5580, 0x04);
            atk_mc5640_write_reg(0x5588, 0x09);
            break;
        }
        case ATK_MC5640_BRIGHTNESS_8:
        {
            atk_mc5640_write_reg(0x5001, 0xFF);
            atk_mc5640_write_reg(0x5587, 0x40);
            atk_mc5640_write_reg(0x5580, 0x04);
            atk_mc5640_write_reg(0x5588, 0x09);
            break;
        }
        default:
        {
            return ATK_MC5640_EINVAL;
        }
    }
    
    return ATK_MC5640_EOK;
}

/**
 * @brief       设置ATK-MC5640模块对比度
 * @param       contrast: 对比度等级
 * @retval      ATK_MC5640_EOK: 成功
 */
uint8_t atk_mc5640_set_contrast(atk_mc5640_contrast_t contrast)
{
    switch (contrast)
    {
        case ATK_MC5640_CONTRAST_0:
        {
            atk_mc5640_write_reg(0x5001, 0xFF);
            atk_mc5640_write_reg(0x5580, 0x04);
            atk_mc5640_write_reg(0x5586, 0x30);
            atk_mc5640_write_reg(0x5585, 0x30);
            atk_mc5640_write_reg(0x5588, 0x41);
            break;
        }
        case ATK_MC5640_CONTRAST_1:
        {
            atk_mc5640_write_reg(0x5001, 0xFF);
            atk_mc5640_write_reg(0x5580, 0x04);
            atk_mc5640_write_reg(0x5586, 0x2C);
            atk_mc5640_write_reg(0x5585, 0x2C);
            atk_mc5640_write_reg(0x5588, 0x41);
            break;
        }
        case ATK_MC5640_CONTRAST_2:
        {
            atk_mc5640_write_reg(0x5001, 0xFF);
            atk_mc5640_write_reg(0x5580, 0x04);
            atk_mc5640_write_reg(0x5586, 0x28);
            atk_mc5640_write_reg(0x5585, 0x28);
            atk_mc5640_write_reg(0x5588, 0x41);
            break;
        }
        case ATK_MC5640_CONTRAST_3:
        {
            atk_mc5640_write_reg(0x5001, 0xFF);
            atk_mc5640_write_reg(0x5580, 0x04);
            atk_mc5640_write_reg(0x5586, 0x24);
            atk_mc5640_write_reg(0x5585, 0x24);
            atk_mc5640_write_reg(0x5588, 0x41);
            break;
        }
        case ATK_MC5640_CONTRAST_4:
        {
            atk_mc5640_write_reg(0x5001, 0xFF);
            atk_mc5640_write_reg(0x5580, 0x04);
            atk_mc5640_write_reg(0x5586, 0x20);
            atk_mc5640_write_reg(0x5585, 0x20);
            atk_mc5640_write_reg(0x5588, 0x41);
            break;
        }
        case ATK_MC5640_CONTRAST_5:
        {
            atk_mc5640_write_reg(0x5001, 0xFF);
            atk_mc5640_write_reg(0x5580, 0x04);
            atk_mc5640_write_reg(0x5586, 0x1C);
            atk_mc5640_write_reg(0x5585, 0x1C);
            atk_mc5640_write_reg(0x5588, 0x41);
            break;
        }
        case ATK_MC5640_CONTRAST_6:
        {
            atk_mc5640_write_reg(0x5001, 0xFF);
            atk_mc5640_write_reg(0x5580, 0x04);
            atk_mc5640_write_reg(0x5586, 0x18);
            atk_mc5640_write_reg(0x5585, 0x18);
            atk_mc5640_write_reg(0x5588, 0x41);
            break;
        }
        case ATK_MC5640_CONTRAST_7:
        {
            atk_mc5640_write_reg(0x5001, 0xFF);
            atk_mc5640_write_reg(0x5580, 0x04);
            atk_mc5640_write_reg(0x5586, 0x14);
            atk_mc5640_write_reg(0x5585, 0x14);
            atk_mc5640_write_reg(0x5588, 0x41);
            break;
        }
        case ATK_MC5640_CONTRAST_8:
        {
            atk_mc5640_write_reg(0x5001, 0xFF);
            atk_mc5640_write_reg(0x5580, 0x04);
            atk_mc5640_write_reg(0x5586, 0x10);
            atk_mc5640_write_reg(0x5585, 0x10);
            atk_mc5640_write_reg(0x5588, 0x41);
            break;
        }
        default:
        {
            return ATK_MC5640_EINVAL;
        }
    }
    
    return ATK_MC5640_EOK;
}

/**
 * @brief       设置ATK-MC5640模块色相
 * @param       hue: 色相等级
 * @retval      ATK_MC5640_EOK: 成功
 */
uint8_t atk_mc5640_set_hue(atk_mc5640_hue_t hue)
{
    switch (hue)
    {
        case ATK_MC5640_HUE_0:
        {
            atk_mc5640_write_reg(0x5001, 0xFF);
            atk_mc5640_write_reg(0x5580, 0x01);
            atk_mc5640_write_reg(0x5581, 0x80);
            atk_mc5640_write_reg(0x5582, 0x00);
            atk_mc5640_write_reg(0x5588, 0x32);
            break;
        }
        case ATK_MC5640_HUE_1:
        {
            atk_mc5640_write_reg(0x5001, 0xFF);
            atk_mc5640_write_reg(0x5580, 0x01);
            atk_mc5640_write_reg(0x5581, 0x6F);
            atk_mc5640_write_reg(0x5582, 0x40);
            atk_mc5640_write_reg(0x5588, 0x32);
            break;
        }
        case ATK_MC5640_HUE_2:
        {
            atk_mc5640_write_reg(0x5001, 0xFF);
            atk_mc5640_write_reg(0x5580, 0x01);
            atk_mc5640_write_reg(0x5581, 0x40);
            atk_mc5640_write_reg(0x5582, 0x6F);
            atk_mc5640_write_reg(0x5588, 0x32);
            break;
        }
        case ATK_MC5640_HUE_3:
        {
            atk_mc5640_write_reg(0x5001, 0xFF);
            atk_mc5640_write_reg(0x5580, 0x01);
            atk_mc5640_write_reg(0x5581, 0x00);
            atk_mc5640_write_reg(0x5582, 0x80);
            atk_mc5640_write_reg(0x5588, 0x02);
            break;
        }
        case ATK_MC5640_HUE_4:
        {
            atk_mc5640_write_reg(0x5001, 0xFF);
            atk_mc5640_write_reg(0x5580, 0x01);
            atk_mc5640_write_reg(0x5581, 0x40);
            atk_mc5640_write_reg(0x5582, 0x6F);
            atk_mc5640_write_reg(0x5588, 0x02);
            break;
        }
        case ATK_MC5640_HUE_5:
        {
            atk_mc5640_write_reg(0x5001, 0xFF);
            atk_mc5640_write_reg(0x5580, 0x01);
            atk_mc5640_write_reg(0x5581, 0x6F);
            atk_mc5640_write_reg(0x5582, 0x40);
            atk_mc5640_write_reg(0x5588, 0x02);
            break;
        }
        case ATK_MC5640_HUE_6:
        {
            atk_mc5640_write_reg(0x5001, 0xFF);
            atk_mc5640_write_reg(0x5580, 0x01);
            atk_mc5640_write_reg(0x5581, 0x80);
            atk_mc5640_write_reg(0x5582, 0x00);
            atk_mc5640_write_reg(0x5588, 0x01);
            break;
        }
        case ATK_MC5640_HUE_7:
        {
            atk_mc5640_write_reg(0x5001, 0xFF);
            atk_mc5640_write_reg(0x5580, 0x01);
            atk_mc5640_write_reg(0x5581, 0x6F);
            atk_mc5640_write_reg(0x5582, 0x40);
            atk_mc5640_write_reg(0x5588, 0x01);
            break;
        }
        case ATK_MC5640_HUE_8:
        {
            atk_mc5640_write_reg(0x5001, 0xFF);
            atk_mc5640_write_reg(0x5580, 0x01);
            atk_mc5640_write_reg(0x5581, 0x40);
            atk_mc5640_write_reg(0x5582, 0x6F);
            atk_mc5640_write_reg(0x5588, 0x01);
            break;
        }
        case ATK_MC5640_HUE_9:
        {
            atk_mc5640_write_reg(0x5001, 0xFF);
            atk_mc5640_write_reg(0x5580, 0x01);
            atk_mc5640_write_reg(0x5581, 0x00);
            atk_mc5640_write_reg(0x5582, 0x80);
            atk_mc5640_write_reg(0x5588, 0x31);
            break;
        }
        case ATK_MC5640_HUE_10:
        {
            atk_mc5640_write_reg(0x5001, 0xFF);
            atk_mc5640_write_reg(0x5580, 0x01);
            atk_mc5640_write_reg(0x5581, 0x40);
            atk_mc5640_write_reg(0x5582, 0x6F);
            atk_mc5640_write_reg(0x5588, 0x31);
            break;
        }
        case ATK_MC5640_HUE_11:
        {
            atk_mc5640_write_reg(0x5001, 0xFF);
            atk_mc5640_write_reg(0x5580, 0x01);
            atk_mc5640_write_reg(0x5581, 0x6F);
            atk_mc5640_write_reg(0x5582, 0x40);
            atk_mc5640_write_reg(0x5588, 0x31);
            break;
        }
        default:
        {
            return ATK_MC5640_EINVAL;
        }
    }
    
    return ATK_MC5640_EOK;
}

/**
 * @brief       设置ATK-MC5640模块特殊效果
 * @param       effect: 特效模式
 * @retval      ATK_MC5640_EOK: 成功
 */
uint8_t atk_mc5640_set_special_effect(atk_mc5640_special_effect_t effect)
{
    switch (effect)
    {
        case ATK_MC5640_SPECIAL_EFFECT_NORMAL:
        {
            atk_mc5640_write_reg(0x5001, 0x7F);
            atk_mc5640_write_reg(0x5580, 0x00);
            break;
        }
        case ATK_MC5640_SPECIAL_EFFECT_BW:
        {
            atk_mc5640_write_reg(0x5001, 0xFF);
            atk_mc5640_write_reg(0x5580, 0x18);
            atk_mc5640_write_reg(0x5583, 0x80);
            atk_mc5640_write_reg(0x5584, 0x80);
            break;
        }
        case ATK_MC5640_SPECIAL_EFFECT_BLUISH:
        {
            atk_mc5640_write_reg(0x5001, 0xFF);
            atk_mc5640_write_reg(0x5580, 0x18);
            atk_mc5640_write_reg(0x5583, 0xA0);
            atk_mc5640_write_reg(0x5584, 0x40);
            break;
        }
        case ATK_MC5640_SPECIAL_EFFECT_SEPIA:
        {
            atk_mc5640_write_reg(0x5001, 0xFF);
            atk_mc5640_write_reg(0x5580, 0x18);
            atk_mc5640_write_reg(0x5583, 0x40);
            atk_mc5640_write_reg(0x5584, 0xA0);
            break;
        }
        case ATK_MC5640_SPECIAL_EFFECT_REDDISH:
        {
            atk_mc5640_write_reg(0x5001, 0xFF);
            atk_mc5640_write_reg(0x5580, 0x18);
            atk_mc5640_write_reg(0x5583, 0x80);
            atk_mc5640_write_reg(0x5584, 0xC0);
            break;
        }
        case ATK_MC5640_SPECIAL_EFFECT_GREENISH:
        {
            atk_mc5640_write_reg(0x5001, 0xFF);
            atk_mc5640_write_reg(0x5580, 0x18);
            atk_mc5640_write_reg(0x5583, 0x60);
            atk_mc5640_write_reg(0x5584, 0x60);
            break;
        }
        case ATK_MC5640_SPECIAL_EFFECT_NEGATIVE:
        {
            atk_mc5640_write_reg(0x5001, 0xFF);
            atk_mc5640_write_reg(0x5580, 0x40);
            break;
        }
        default:
        {
            return ATK_MC5640_EINVAL;
        }
    }
    
    return ATK_MC5640_EOK;
}

/**
 * @brief       设置ATK-MC5640模块曝光度
 * @param       level: 曝光等级
 * @retval      ATK_MC5640_EOK: 成功
 */
uint8_t atk_mc5640_set_exposure_level(atk_mc5640_exposure_level_t level)
{
    switch (level)
    {
        case ATK_MC5640_EXPOSURE_LEVEL_0:
        {
            atk_mc5640_write_reg(0x3A0F, 0x10);
            atk_mc5640_write_reg(0x3A10, 0x08);
            atk_mc5640_write_reg(0x3A1B, 0x10);
            atk_mc5640_write_reg(0x3A1E, 0x08);
            atk_mc5640_write_reg(0x3A11, 0x20);
            atk_mc5640_write_reg(0x3A1F, 0x10);
            break;
        }
        case ATK_MC5640_EXPOSURE_LEVEL_1:
        {
            atk_mc5640_write_reg(0x3A0F, 0x18);
            atk_mc5640_write_reg(0x3A10, 0x10);
            atk_mc5640_write_reg(0x3A1B, 0x18);
            atk_mc5640_write_reg(0x3A1E, 0x10);
            atk_mc5640_write_reg(0x3A11, 0x30);
            atk_mc5640_write_reg(0x3A1F, 0x10);
            break;
        }
        case ATK_MC5640_EXPOSURE_LEVEL_2:
        {
            atk_mc5640_write_reg(0x3A0F, 0x20);
            atk_mc5640_write_reg(0x3A10, 0x18);
            atk_mc5640_write_reg(0x3A11, 0x41);
            atk_mc5640_write_reg(0x3A1B, 0x20);
            atk_mc5640_write_reg(0x3A1E, 0x18);
            atk_mc5640_write_reg(0x3A1F, 0x10);
            break;
        }
        case ATK_MC5640_EXPOSURE_LEVEL_3:
        {
            atk_mc5640_write_reg(0x3A0F, 0x28);
            atk_mc5640_write_reg(0x3A10, 0x20);
            atk_mc5640_write_reg(0x3A11, 0x51);
            atk_mc5640_write_reg(0x3A1B, 0x28);
            atk_mc5640_write_reg(0x3A1E, 0x20);
            atk_mc5640_write_reg(0x3A1F, 0x10);
            break;
        }
        case ATK_MC5640_EXPOSURE_LEVEL_4:
        {
            atk_mc5640_write_reg(0x3A0F, 0x30);
            atk_mc5640_write_reg(0x3A10, 0x28);
            atk_mc5640_write_reg(0x3A11, 0x61);
            atk_mc5640_write_reg(0x3A1B, 0x30);
            atk_mc5640_write_reg(0x3A1E, 0x28);
            atk_mc5640_write_reg(0x3A1F, 0x10);
            break;
        }
        case ATK_MC5640_EXPOSURE_LEVEL_5:
        {
            atk_mc5640_write_reg(0x3A0F, 0x38);
            atk_mc5640_write_reg(0x3A10, 0x30);
            atk_mc5640_write_reg(0x3A11, 0x61);
            atk_mc5640_write_reg(0x3A1B, 0x38);
            atk_mc5640_write_reg(0x3A1E, 0x30);
            atk_mc5640_write_reg(0x3A1F, 0x10);
            break;
        }
        case ATK_MC5640_EXPOSURE_LEVEL_6:
        {
            atk_mc5640_write_reg(0x3A0F, 0x40);
            atk_mc5640_write_reg(0x3A10, 0x38);
            atk_mc5640_write_reg(0x3A11, 0x71);
            atk_mc5640_write_reg(0x3A1B, 0x40);
            atk_mc5640_write_reg(0x3A1E, 0x38);
            atk_mc5640_write_reg(0x3A1F, 0x10);
            break;
        }
        case ATK_MC5640_EXPOSURE_LEVEL_7:
        {
            atk_mc5640_write_reg(0x3A0F, 0x48);
            atk_mc5640_write_reg(0x3A10, 0x40);
            atk_mc5640_write_reg(0x3A11, 0x80);
            atk_mc5640_write_reg(0x3A1B, 0x48);
            atk_mc5640_write_reg(0x3A1E, 0x40);
            atk_mc5640_write_reg(0x3A1F, 0x20);
            break;
        }
        case ATK_MC5640_EXPOSURE_LEVEL_8:
        {
            atk_mc5640_write_reg(0x3A0F, 0x50);
            atk_mc5640_write_reg(0x3A10, 0x48);
            atk_mc5640_write_reg(0x3A11, 0x90);
            atk_mc5640_write_reg(0x3A1B, 0x50);
            atk_mc5640_write_reg(0x3A1E, 0x48);
            atk_mc5640_write_reg(0x3A1F, 0x20);
            break;
        }
        case ATK_MC5640_EXPOSURE_LEVEL_9:
        {
            atk_mc5640_write_reg(0x3A0F, 0x58);
            atk_mc5640_write_reg(0x3A10, 0x50);
            atk_mc5640_write_reg(0x3A11, 0x91);
            atk_mc5640_write_reg(0x3A1B, 0x58);
            atk_mc5640_write_reg(0x3A1E, 0x50);
            atk_mc5640_write_reg(0x3A1F, 0x20);
            break;
        }
        case ATK_MC5640_EXPOSURE_LEVEL_10:
        {
            atk_mc5640_write_reg(0x3A0F, 0x60);
            atk_mc5640_write_reg(0x3A10, 0x58);
            atk_mc5640_write_reg(0x3A11, 0xA0);
            atk_mc5640_write_reg(0x3A1B, 0x60);
            atk_mc5640_write_reg(0x3A1E, 0x58);
            atk_mc5640_write_reg(0x3A1F, 0x20);
            break;
        }
        default:
        {
            return ATK_MC5640_EINVAL;
        }
    }
    
    return ATK_MC5640_EOK;
}

/**
 * @brief       设置ATK-MC5640模块锐度
 * @param       sharpness: 锐度等级
 * @retval      ATK_MC5640_EOK: 成功
 */
uint8_t atk_mc5640_set_sharpness_level(atk_mc5640_sharpness_t sharpness)
{
    switch (sharpness)
    {
        case ATK_MC5640_SHARPNESS_OFF:
        {
            atk_mc5640_write_reg(0x5308, 0x65);
            atk_mc5640_write_reg(0x5302, 0x00);
            break;
        }
        case ATK_MC5640_SHARPNESS_1:
        {
            atk_mc5640_write_reg(0x5308, 0x65);
            atk_mc5640_write_reg(0x5302, 0x02);
            break;
        }
        case ATK_MC5640_SHARPNESS_2:
        {
            atk_mc5640_write_reg(0x5308, 0x65);
            atk_mc5640_write_reg(0x5302, 0x04);
            break;
        }
        case ATK_MC5640_SHARPNESS_3:
        {
            atk_mc5640_write_reg(0x5308, 0x65);
            atk_mc5640_write_reg(0x5302, 0x08);
            break;
        }
        case ATK_MC5640_SHARPNESS_4:
        {
            atk_mc5640_write_reg(0x5308, 0x65);
            atk_mc5640_write_reg(0x5302, 0x0C);
            break;
        }
        case ATK_MC5640_SHARPNESS_5:
        {
            atk_mc5640_write_reg(0x5308, 0x65);
            atk_mc5640_write_reg(0x5302, 0x10);
            break;
        }
        case ATK_MC5640_SHARPNESS_6:
        {
            atk_mc5640_write_reg(0x5308, 0x65);
            atk_mc5640_write_reg(0x5302, 0x14);
            break;
        }
        case ATK_MC5640_SHARPNESS_7:
        {
            atk_mc5640_write_reg(0x5308, 0x65);
            atk_mc5640_write_reg(0x5302, 0x18);
            break;
        }
        case ATK_MC5640_SHARPNESS_8:
        {
            atk_mc5640_write_reg(0x5308, 0x65);
            atk_mc5640_write_reg(0x5302, 0x20);
            break;
        }
        case ATK_MC5640_SHARPNESS_AUTO:
        {
            atk_mc5640_write_reg(0x5308, 0x25);
            atk_mc5640_write_reg(0x5300, 0x08);
            atk_mc5640_write_reg(0x5301, 0x30);
            atk_mc5640_write_reg(0x5302, 0x10);
            atk_mc5640_write_reg(0x5303, 0x00);
            atk_mc5640_write_reg(0x5309, 0x08);
            atk_mc5640_write_reg(0x530A, 0x30);
            atk_mc5640_write_reg(0x530B, 0x04);
            atk_mc5640_write_reg(0x530C, 0x06);
            break;
        }
        default:
        {
            return ATK_MC5640_EINVAL;
        }
    }
    
    return ATK_MC5640_EOK;
}

/**
 * @brief       设置ATK-MC5640模块镜像/翻转
 * @param       mirror_flip: 镜像/翻转模式
 * @retval      ATK_MC5640_EOK: 成功
 */
uint8_t atk_mc5640_set_mirror_flip(atk_mc5640_mirror_flip_t mirror_flip)
{
    uint8_t reg3820;
    uint8_t reg3821;
    
    switch (mirror_flip)
    {
        case ATK_MC5640_MIRROR_FLIP_0:
        {
            reg3820 = atk_mc5640_read_reg(0x3820);
            reg3820 = reg3820 & 0xF9;
            reg3820 = reg3820 | 0x00;
            atk_mc5640_write_reg(0x3820, reg3820);
            reg3821 = atk_mc5640_read_reg(0x3821);
            reg3821 = reg3821 & 0xF9;
            reg3821 = reg3821 | 0x06;
            atk_mc5640_write_reg(0x3821, reg3821);
            break;
        }
        case ATK_MC5640_MIRROR_FLIP_1:
        {
            reg3820 = atk_mc5640_read_reg(0x3820);
            reg3820 = reg3820 & 0xF9;
            reg3820 = reg3820 | 0x06;
            atk_mc5640_write_reg(0x3820, reg3820);
            reg3821 = atk_mc5640_read_reg(0x3821);
            reg3821 = reg3821 & 0xF9;
            reg3821 = reg3821 | 0x00;
            atk_mc5640_write_reg(0x3821, reg3821);
            break;
        }
        case ATK_MC5640_MIRROR_FLIP_2:
        {
            reg3820 = atk_mc5640_read_reg(0x3820);
            reg3820 = reg3820 & 0xF9;
            reg3820 = reg3820 | 0x06;
            atk_mc5640_write_reg(0x3820, reg3820);
            reg3821 = atk_mc5640_read_reg(0x3821);
            reg3821 = reg3821 & 0xF9;
            reg3821 = reg3821 | 0x06;
            atk_mc5640_write_reg(0x3821, reg3821);
            break;
        }
        case ATK_MC5640_MIRROR_FLIP_3:
        {
            reg3820 = atk_mc5640_read_reg(0x3820);
            reg3820 = reg3820 & 0xF9;
            reg3820 = reg3820 | 0x00;
            atk_mc5640_write_reg(0x3820, reg3820);
            reg3821 = atk_mc5640_read_reg(0x3821);
            reg3821 = reg3821 & 0xF9;
            reg3821 = reg3821 | 0x00;
            atk_mc5640_write_reg(0x3821, reg3821);
            break;
        }
        default:
        {
            return ATK_MC5640_EINVAL;
        }
    }
    
    return ATK_MC5640_EOK;
}

/**
 * @brief       设置ATK-MC5640模块测试图案
 * @param       pattern: 测试图案模式
 * @retval      ATK_MC5640_EOK: 成功
 */
uint8_t atk_mc5640_set_test_pattern(atk_mc5640_test_pattern_t pattern)
{
    switch (pattern)
    {
        case ATK_MC5640_TEST_PATTERN_OFF:
        {
            atk_mc5640_write_reg(0x503D, 0x00);
            atk_mc5640_write_reg(0x4741, 0x00);
            break;
        }
        case ATK_MC5640_TEST_PATTERN_COLOR_BAR:
        {
            atk_mc5640_write_reg(0x503D, 0x80);
            atk_mc5640_write_reg(0x4741, 0x00);
            break;
        }
        case ATK_MC5640_TEST_PATTERN_COLOR_SQUARE:
        {
            atk_mc5640_write_reg(0x503D, 0x82);
            atk_mc5640_write_reg(0x4741, 0x00);
            break;
        }
        default:
        {
            return ATK_MC5640_EINVAL;
        }
    }
    
    return ATK_MC5640_EOK;
}

/**
 * @brief       设置ATK-MC5640模块输出图像格式
 * @param       format: 输出格式 (RGB565/JPEG)
 * @retval      ATK_MC5640_EOK: 成功
 */
uint8_t atk_mc5640_set_output_format(atk_mc5640_output_format_t format)
{
    uint32_t cfg_index;
    
    switch (format)
    {
        case ATK_MC5640_OUTPUT_FORMAT_RGB565:
        {
            for (cfg_index=0; cfg_index<sizeof(atk_mc5640_rgb565_cfg)/sizeof(atk_mc5640_rgb565_cfg[0]); cfg_index++)
            {
                atk_mc5640_write_reg(atk_mc5640_rgb565_cfg[cfg_index].reg, atk_mc5640_rgb565_cfg[cfg_index].dat);
            }
            break;
        }
        case ATK_MC5640_OUTPUT_FORMAT_JPEG:
        {
            for (cfg_index=0; cfg_index<sizeof(atk_mc5640_jpeg_cfg)/sizeof(atk_mc5640_jpeg_cfg[0]); cfg_index++)
            {
                atk_mc5640_write_reg(atk_mc5640_jpeg_cfg[cfg_index].reg, atk_mc5640_jpeg_cfg[cfg_index].dat);
            }
            break;
        }
        default:
        {
            return ATK_MC5640_EINVAL;
        }
    }
    
    return ATK_MC5640_EOK;
}

/**
 * @brief       设置ATK-MC5640模块ISP输入窗口尺寸
 * @param       x, y, width, height: 窗口参数
 * @retval      ATK_MC5640_EOK: 成功
 */
uint8_t atk_mc5640_set_isp_input_window(uint16_t x, uint16_t y, uint16_t width, uint16_t height)
{
    uint8_t reg3800;
    uint8_t reg3801;
    uint8_t reg3802;
    uint8_t reg3803;
    uint8_t reg3804;
    uint8_t reg3805;
    uint8_t reg3806;
    uint8_t reg3807;
    uint16_t x_end;
    uint16_t y_end;
    
    x_end = x + width;
    y_end = y + height;
    
    if ((x_end > ATK_MC5640_ISP_INPUT_WIDTH_MAX) || (y_end > ATK_MC5640_ISP_INPUT_HEIGHT_MAX))
    {
        return ATK_MC5640_EINVAL;
    }
    
    reg3800 = atk_mc5640_read_reg(0x3800);
    reg3802 = atk_mc5640_read_reg(0x3802);
    reg3804 = atk_mc5640_read_reg(0x3804);
    reg3806 = atk_mc5640_read_reg(0x3806);
    
    reg3800 &= ~0x0F;
    reg3800 |= (uint8_t)(x >> 8) & 0x0F;
    reg3801 = (uint8_t)x & 0xFF;
    reg3802 &= ~0x0F;
    reg3802 |= (uint8_t)(y >> 8) & 0x0F;
    reg3803 = (uint8_t)y & 0xFF;
    reg3804 &= ~0x0F;
    reg3804 |= (uint8_t)(x_end >> 8) & 0x0F;
    reg3805 = (uint8_t)x_end & 0xFF;
    reg3806 &= ~0x07;
    reg3806 |= (uint8_t)(y_end >> 8) & 0x07;
    reg3807 = (uint8_t)y_end & 0xFF;
    
    atk_mc5640_write_reg(0x3212, 0x03);
    atk_mc5640_write_reg(0x3800, reg3800);
    atk_mc5640_write_reg(0x3801, reg3801);
    atk_mc5640_write_reg(0x3802, reg3802);
    atk_mc5640_write_reg(0x3803, reg3803);
    atk_mc5640_write_reg(0x3804, reg3804);
    atk_mc5640_write_reg(0x3805, reg3805);
    atk_mc5640_write_reg(0x3806, reg3806);
    atk_mc5640_write_reg(0x3807, reg3807);
    atk_mc5640_write_reg(0x3212, 0x13);
    atk_mc5640_write_reg(0x3212, 0xA3);
    
    atk_mc5640_get_isp_input_size();
    
    return ATK_MC5640_EOK;
}

/**
 * @brief       设置ATK-MC5640模块预缩放窗口偏移
 * @param       x_offset, y_offset: 偏移量
 * @retval      ATK_MC5640_EOK: 成功
 */
uint8_t atk_mc5640_set_pre_scaling_window(uint16_t x_offset, uint16_t y_offset)
{
    uint8_t reg3810;
    uint8_t reg3811;
    uint8_t reg3812;
    uint8_t reg3813;
    
    reg3810 = (uint8_t)(x_offset >> 8) & 0x0F;
    reg3811 = (uint8_t)x_offset & 0xFF;
    reg3812 = (uint8_t)(y_offset >> 8) & 0x07;
    reg3813 = (uint8_t)y_offset & 0xFF;
    
    atk_mc5640_write_reg(0x3212, 0x03);
    atk_mc5640_write_reg(0x3810, reg3810);
    atk_mc5640_write_reg(0x3811, reg3811);
    atk_mc5640_write_reg(0x3812, reg3812);
    atk_mc5640_write_reg(0x3813, reg3813);
    atk_mc5640_write_reg(0x3212, 0x13);
    atk_mc5640_write_reg(0x3212, 0xA3);
    
    atk_mc5640_get_pre_scaling_size();
    
    return ATK_MC5640_EOK;
}

/**
 * @brief       设置ATK-MC5640模块输出图像尺寸
 * @param       width, height: 目标尺寸
 * @retval      ATK_MC5640_EOK: 成功
 */
uint8_t atk_mc5640_set_output_size(uint16_t width, uint16_t height)
{
    uint8_t reg3808;
    uint8_t reg3809;
    uint8_t reg380A;
    uint8_t reg380B;
    
    reg3808 = atk_mc5640_read_reg(0x3808);
    reg380A = atk_mc5640_read_reg(0x380A);
    
    reg3808 &= ~0x0F;
    reg3808 |= (uint8_t)(width >> 8) & 0x0F;
    reg3809 = (uint8_t)width & 0xFF;
    reg380A &= ~0x07;
    reg380A |= (uint8_t)(height >> 8) & 0x07;
    reg380B = (uint8_t)height & 0xFF;
    
    atk_mc5640_write_reg(0x3212, 0x03);
    atk_mc5640_write_reg(0x3808, reg3808);
    atk_mc5640_write_reg(0x3809, reg3809);
    atk_mc5640_write_reg(0x380A, reg380A);
    atk_mc5640_write_reg(0x380B, reg380B);
    atk_mc5640_write_reg(0x3212, 0x13);
    atk_mc5640_write_reg(0x3212, 0xA3);
    
    atk_mc5640_get_output_size();
    
    return ATK_MC5640_EOK;
}

/**
 * @brief       获取ATK-MC5640模块输出的一帧图像数据 (HAL库适配版)
 * @param       dts_addr : 帧数据的接收缓冲的首地址
 * @param       type: 传输类型 (此处简化逻辑，主要依赖 CubeMX 配置的 DMA 参数)
 * @param       before_transfer: 传输前回调函数
 * @retval      ATK_MC5640_EOK: 成功
 */
uint8_t atk_mc5640_get_frame(uint32_t dts_addr, atk_mc5640_get_type_t type, void (*before_transfer)(void))
{
    /*
     * 注意：在 HAL 库中，DMA 的位宽和增量模式通常在 CubeMX 初始化时已定死。
     * 如果需要动态修改 DMA 参数 (如 type 参数所示)，需要直接操作 DMA 句柄。
     * 下面的代码尝试根据 type 修改 hdcmi.DMA_Handle 的配置。
     */

    uint32_t meminc;
    uint32_t memdataalignment;
    uint32_t len;
    
    /* 计算长度和配置 DMA 参数 */
    switch (type)
    {
        case ATK_MC5640_GET_TYPE_DTS_8B_NOINC:
            meminc = DMA_MINC_DISABLE;
            memdataalignment = DMA_MDATAALIGN_BYTE;
            len = (g_atk_mc5640_sta.output.width * g_atk_mc5640_sta.output.height) / 1;
            break;
        case ATK_MC5640_GET_TYPE_DTS_8B_INC:
            meminc = DMA_MINC_ENABLE;
            memdataalignment = DMA_MDATAALIGN_BYTE;
            len = (g_atk_mc5640_sta.output.width * g_atk_mc5640_sta.output.height) / 1;
            break;
        case ATK_MC5640_GET_TYPE_DTS_16B_NOINC:
            meminc = DMA_MINC_DISABLE;
            memdataalignment = DMA_MDATAALIGN_HALFWORD;
            len = (g_atk_mc5640_sta.output.width * g_atk_mc5640_sta.output.height) / 2;
            break;
        case ATK_MC5640_GET_TYPE_DTS_16B_INC:
            meminc = DMA_MINC_ENABLE;
            memdataalignment = DMA_MDATAALIGN_HALFWORD;
            len = (g_atk_mc5640_sta.output.width * g_atk_mc5640_sta.output.height) / 2;
            break;
        case ATK_MC5640_GET_TYPE_DTS_32B_NOINC:
            meminc = DMA_MINC_DISABLE;
            memdataalignment = DMA_MDATAALIGN_WORD;
            len = (g_atk_mc5640_sta.output.width * g_atk_mc5640_sta.output.height) / 4;
            break;
        case ATK_MC5640_GET_TYPE_DTS_32B_INC:
            meminc = DMA_MINC_ENABLE;
            memdataalignment = DMA_MDATAALIGN_WORD;
            len = (g_atk_mc5640_sta.output.width * g_atk_mc5640_sta.output.height) / 4;
            break;
        default:
            return ATK_MC5640_EINVAL;
    }
    
    /* 动态修改 DMA 配置 (需要先停止 DMA) */
    HAL_DMA_Abort(hdcmi.DMA_Handle);
    hdcmi.DMA_Handle->Init.MemInc = meminc;
    hdcmi.DMA_Handle->Init.MemDataAlignment = memdataalignment;
    HAL_DMA_Init(hdcmi.DMA_Handle);

    /* 执行传输前回调 */
    if (before_transfer != NULL)
    {
        before_transfer();
    }
    
    /* 启动 DCMI 传输 (Snapshot 模式: 拍一张) */
    /* 注意: HAL_DCMI_Start_DMA 内部会自动使能 DCMI 和 DMA */
    g_ov5640_frame_cplt = 0; // 清除完成标志
    HAL_DCMI_Start_DMA(&hdcmi, DCMI_MODE_SNAPSHOT, dts_addr, len);

    /* 等待传输完成 (阻塞式，或者在应用层通过查询 g_ov5640_frame_cplt 来实现非阻塞) */
    /* 这里为了保持原函数接口行为，使用阻塞等待 */
    while (g_ov5640_frame_cplt == 0)
    {
        // 可以加入超时退出机制
    }

    HAL_DCMI_Stop(&hdcmi);
    
    return ATK_MC5640_EOK;
}

/**
 * @brief  应用例程中的标准画质增强配置
 */
void OV5640_Apply_Best_Settings(void)
{
    atk_mc5640_set_light_mode(ATK_MC5640_LIGHT_MODE_ADVANCED_AWB);
    atk_mc5640_set_color_saturation(ATK_MC5640_COLOR_SATURATION_4);
    atk_mc5640_set_brightness(ATK_MC5640_BRIGHTNESS_4);
    atk_mc5640_set_contrast(ATK_MC5640_CONTRAST_4);
    atk_mc5640_set_hue(ATK_MC5640_HUE_6);
    atk_mc5640_set_special_effect(ATK_MC5640_SPECIAL_EFFECT_NORMAL);
    atk_mc5640_set_exposure_level(ATK_MC5640_EXPOSURE_LEVEL_5);
    atk_mc5640_set_sharpness_level(ATK_MC5640_SHARPNESS_OFF);
    // 注意：这里我们根据沙盘视角，可能需要微调镜像/翻转
    atk_mc5640_set_mirror_flip(ATK_MC5640_MIRROR_FLIP_1);
}
/**
 * @brief       HAL库 DCMI 帧中断回调函数
 * @param       hdcmi: DCMI句柄
 * @retval      无
 */
/*
// ❌ 删除或注释掉这段冲突的代码
void HAL_DCMI_FrameEventCallback(DCMI_HandleTypeDef *hdcmi)
{
    g_ov5640_frame_cplt = 1;
}
*/
