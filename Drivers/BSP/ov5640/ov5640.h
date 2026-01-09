/**
 ****************************************************************************************************
 * @file        atk_mc5640.h
 * @author      ����ԭ���Ŷ�(ALIENTEK)
 * @version     V1.0
 * @date        2022-06-21
 * @brief       ATK-MC5640ģ����������
 * @license     Copyright (c) 2020-2032, ������������ӿƼ����޹�˾
 ****************************************************************************************************
 * @attention
 *
 * ʵ��ƽ̨:����ԭ�� ������ H743������
 * ������Ƶ:www.yuanzige.com
 * ������̳:www.openedv.com
 * ��˾��ַ:www.alientek.com
 * �����ַ:openedv.taobao.com
 *
 ****************************************************************************************************
 */

#ifndef __OV5640_H
#define __OV5640_H

#include <stdint.h>

/*
 * 定义闪光灯控制方式
 * 1: 通过 OV5640 寄存器控制 (推荐，无需额外 GPIO)
 * 0: 通过 MCU GPIO 控制 (需要您在 CubeMX 配置 FLASH 引脚)
 */
#define ATK_MC5640_LED_CTL_BY_OV5640    1

/*
 * ATK-MC5640 SCCB 通讯地址
 * 注意：HAL库使用 8-bit 地址 (7-bit 地址 0x3C 左移一位 -> 0x78)
 */
#define ATK_MC5640_SCCB_ADDR            0x78

/* ATK-MC5640模块 ISP 输入窗口最大尺寸 (保持不变) */
#define ATK_MC5640_ISP_INPUT_WIDTH_MAX  0x0A3F // 2623
#define ATK_MC5640_ISP_INPUT_HEIGHT_MAX 0x06A9 // 1705

/* ATK-MC5640模块灯光模式枚举 */
typedef enum
{
    ATK_MC5640_LIGHT_MODE_ADVANCED_AWB = 0x00,  /* Advanced AWB */
    ATK_MC5640_LIGHT_MODE_SIMPLE_AWB,           /* Simple AWB */
    ATK_MC5640_LIGHT_MODE_MANUAL_DAY,           /* Manual day */
    ATK_MC5640_LIGHT_MODE_MANUAL_A,             /* Manual A */
    ATK_MC5640_LIGHT_MODE_MANUAL_CWF,           /* Manual cwf */
    ATK_MC5640_LIGHT_MODE_MANUAL_CLOUDY         /* Manual cloudy */
} atk_mc5640_light_mode_t;

/* ATK-MC5640模块色彩饱和度枚举 */
typedef enum
{
    ATK_MC5640_COLOR_SATURATION_0 = 0x00,       /* +4 */
    ATK_MC5640_COLOR_SATURATION_1,              /* +3 */
    ATK_MC5640_COLOR_SATURATION_2,              /* +2 */
    ATK_MC5640_COLOR_SATURATION_3,              /* +1 */
    ATK_MC5640_COLOR_SATURATION_4,              /* 0 */
    ATK_MC5640_COLOR_SATURATION_5,              /* -1 */
    ATK_MC5640_COLOR_SATURATION_6,              /* -2 */
    ATK_MC5640_COLOR_SATURATION_7,              /* -3 */
    ATK_MC5640_COLOR_SATURATION_8,              /* -4 */
} atk_mc5640_color_saturation_t;

/* ATK-MC5640模块亮度枚举 */
typedef enum
{
    ATK_MC5640_BRIGHTNESS_0 = 0x00,             /* +4 */
    ATK_MC5640_BRIGHTNESS_1,                    /* +3 */
    ATK_MC5640_BRIGHTNESS_2,                    /* +2 */
    ATK_MC5640_BRIGHTNESS_3,                    /* +1 */
    ATK_MC5640_BRIGHTNESS_4,                    /* 0 */
    ATK_MC5640_BRIGHTNESS_5,                    /* -1 */
    ATK_MC5640_BRIGHTNESS_6,                    /* -2 */
    ATK_MC5640_BRIGHTNESS_7,                    /* -3 */
    ATK_MC5640_BRIGHTNESS_8,                    /* -4 */
} atk_mc5640_brightness_t;

/* ATK-MC5640模块对比度枚举 */
typedef enum
{
    ATK_MC5640_CONTRAST_0 = 0x00,               /* +4 */
    ATK_MC5640_CONTRAST_1,                      /* +3 */
    ATK_MC5640_CONTRAST_2,                      /* +2 */
    ATK_MC5640_CONTRAST_3,                      /* +1 */
    ATK_MC5640_CONTRAST_4,                      /* 0 */
    ATK_MC5640_CONTRAST_5,                      /* -1 */
    ATK_MC5640_CONTRAST_6,                      /* -2 */
    ATK_MC5640_CONTRAST_7,                      /* -3 */
    ATK_MC5640_CONTRAST_8,                      /* -4 */
} atk_mc5640_contrast_t;

/* ATK-MC5640模块色相枚举 */
typedef enum
{
    ATK_MC5640_HUE_0 = 0x00,                    /* -180 degree */
    ATK_MC5640_HUE_1,                           /* -150 degree */
    ATK_MC5640_HUE_2,                           /* -120 degree */
    ATK_MC5640_HUE_3,                           /* -90 degree */
    ATK_MC5640_HUE_4,                           /* -60 degree */
    ATK_MC5640_HUE_5,                           /* -30 degree */
    ATK_MC5640_HUE_6,                           /* 0 degree */
    ATK_MC5640_HUE_7,                           /* +30 degree */
    ATK_MC5640_HUE_8,                           /* +60 degree */
    ATK_MC5640_HUE_9,                           /* +90 degree */
    ATK_MC5640_HUE_10,                          /* +120 degree */
    ATK_MC5640_HUE_11,                          /* +150 degree */
} atk_mc5640_hue_t;

/* ATK-MC5640模块特殊效果枚举 */
typedef enum
{
    ATK_MC5640_SPECIAL_EFFECT_NORMAL = 0x00,    /* Normal */
    ATK_MC5640_SPECIAL_EFFECT_BW,               /* B&W */
    ATK_MC5640_SPECIAL_EFFECT_BLUISH,           /* Bluish */
    ATK_MC5640_SPECIAL_EFFECT_SEPIA,            /* Sepia */
    ATK_MC5640_SPECIAL_EFFECT_REDDISH,          /* Reddish */
    ATK_MC5640_SPECIAL_EFFECT_GREENISH,         /* Greenish */
    ATK_MC5640_SPECIAL_EFFECT_NEGATIVE,         /* Negative */
} atk_mc5640_special_effect_t;

/* ATK-MC5640模块曝光度枚举 */
typedef enum
{
    ATK_MC5640_EXPOSURE_LEVEL_0 = 0x00,         /* -1.7EV */
    ATK_MC5640_EXPOSURE_LEVEL_1,                /* -1.3EV */
    ATK_MC5640_EXPOSURE_LEVEL_2,                /* -1.0EV */
    ATK_MC5640_EXPOSURE_LEVEL_3,                /* -0.7EV */
    ATK_MC5640_EXPOSURE_LEVEL_4,                /* -0.3EV */
    ATK_MC5640_EXPOSURE_LEVEL_5,                /* default */
    ATK_MC5640_EXPOSURE_LEVEL_6,                /* 0.3EV */
    ATK_MC5640_EXPOSURE_LEVEL_7,                /* 0.7EV */
    ATK_MC5640_EXPOSURE_LEVEL_8,                /* 1.0EV */
    ATK_MC5640_EXPOSURE_LEVEL_9,                /* 1.3EV */
    ATK_MC5640_EXPOSURE_LEVEL_10,               /* 1.7EV */
} atk_mc5640_exposure_level_t;

/* ATK-MC5640模块锐度枚举 */
typedef enum
{
    ATK_MC5640_SHARPNESS_OFF = 0x00,            /* Sharpness OFF */
    ATK_MC5640_SHARPNESS_1,                     /* Sharpness 1 */
    ATK_MC5640_SHARPNESS_2,                     /* Sharpness 2 */
    ATK_MC5640_SHARPNESS_3,                     /* Sharpness 3 */
    ATK_MC5640_SHARPNESS_4,                     /* Sharpness 4 */
    ATK_MC5640_SHARPNESS_5,                     /* Sharpness 5 */
    ATK_MC5640_SHARPNESS_6,                     /* Sharpness 6 */
    ATK_MC5640_SHARPNESS_7,                     /* Sharpness 7 */
    ATK_MC5640_SHARPNESS_8,                     /* Sharpness 8 */
    ATK_MC5640_SHARPNESS_AUTO,                  /* Sharpness Auto */
} atk_mc5640_sharpness_t;

/* ATK-MC5640模块镜像/翻转枚举 */
typedef enum
{
    ATK_MC5640_MIRROR_FLIP_0 = 0x00,            /* MIRROR */
    ATK_MC5640_MIRROR_FLIP_1,                   /* FLIP */
    ATK_MC5640_MIRROR_FLIP_2,                   /* MIRROR & FLIP */
    ATK_MC5640_MIRROR_FLIP_3,                   /* Normal */
} atk_mc5640_mirror_flip_t;

/* ATK-MC5640模块测试图案枚举 */
typedef enum
{
    ATK_MC5640_TEST_PATTERN_OFF = 0x00,         /* OFF */
    ATK_MC5640_TEST_PATTERN_COLOR_BAR,          /* Color bar */
    ATK_MC5640_TEST_PATTERN_COLOR_SQUARE,       /* Color square */
} atk_mc5640_test_pattern_t;

/* ATK-MC5640输出图像格式枚举 */
typedef enum
{
    ATK_MC5640_OUTPUT_FORMAT_RGB565 = 0x00,     /* RGB565 */
    ATK_MC5640_OUTPUT_FORMAT_JPEG,              /* JPEG */
} atk_mc5640_output_format_t;

/* ATK-MC5640获取帧数据方式枚举 */
typedef enum
{
    ATK_MC5640_GET_TYPE_DTS_8B_NOINC = 0x00,    /* 图像数据以字节方式写入目的地址，目的地址固定不变 */
    ATK_MC5640_GET_TYPE_DTS_8B_INC,             /* 图像数据以字节方式写入目的地址，目的地址自动增加 */
    ATK_MC5640_GET_TYPE_DTS_16B_NOINC,          /* 图像数据以半字方式写入目的地址，目的地址固定不变 */
    ATK_MC5640_GET_TYPE_DTS_16B_INC,            /* 图像数据以半字方式写入目的地址，目的地址自动增加 */
    ATK_MC5640_GET_TYPE_DTS_32B_NOINC,          /* 图像数据以字方式写入目的地址，目的地址固定不变 */
    ATK_MC5640_GET_TYPE_DTS_32B_INC,            /* 图像数据以字方式写入目的地址，目的地址自动增加 */
} atk_mc5640_get_type_t;

/* ========================================================================== */
/*                               错误代码与函数声明                            */
/* ========================================================================== */

/* 错误代码 */
#define ATK_MC5640_EOK      0   /* 没有错误 */
#define ATK_MC5640_ERROR    1   /* 错误 */
#define ATK_MC5640_EINVAL   2   /* 非法参数 */
#define ATK_MC5640_ENOMEM   3   /* 内存不足 */
#define ATK_MC5640_EEMPTY   4   /* 资源为空 */
#define ATK_MC5640_ETIMEOUT 5   /* 操作超时 */

/* 操作函数 */
uint8_t atk_mc5640_init(void);                                                                              /* 初始化ATK-MC5640模块 */
uint8_t atk_mc5640_auto_focus_init(void);                                                                   /* 初始化ATK-MC5640模块自动对焦 */
uint8_t atk_mc5640_auto_focus_once(void);                                                                   /* ATK-MC5640模块自动对焦一次 */
uint8_t atk_mc5640_auto_focus_continuance(void);                                                            /* ATK-MC5640模块持续自动对焦 */
void atk_mc5640_led_on(void);                                                                               /* 开启ATK-MC5640模块闪光灯 */
void atk_mc5640_led_off(void);                                                                              /* 关闭ATK-MC5640模块闪光灯 */
uint8_t atk_mc5640_set_light_mode(atk_mc5640_light_mode_t mode);                                            /* 设置ATK-MC5640模块灯光模式 */
uint8_t atk_mc5640_set_color_saturation(atk_mc5640_color_saturation_t saturation);                          /* 设置ATK-MC5640模块色彩饱和度 */
uint8_t atk_mc5640_set_brightness(atk_mc5640_brightness_t brightness);                                      /* 设置ATK-MC5640模块亮度 */
uint8_t atk_mc5640_set_contrast(atk_mc5640_contrast_t contrast);                                            /* 设置ATK-MC5640模块对比度 */
uint8_t atk_mc5640_set_hue(atk_mc5640_hue_t hue);                                                           /* 设置ATK-MC5640模块色相 */
uint8_t atk_mc5640_set_special_effect(atk_mc5640_special_effect_t effect);                                  /* 设置ATK-MC5640模块特殊效果 */
uint8_t atk_mc5640_set_exposure_level(atk_mc5640_exposure_level_t level);                                   /* 设置ATK-MC5640模块曝光度 */
uint8_t atk_mc5640_set_sharpness_level(atk_mc5640_sharpness_t sharpness);                                   /* 设置ATK-MC5640模块锐度 */
uint8_t atk_mc5640_set_mirror_flip(atk_mc5640_mirror_flip_t mirror_flip);                                   /* 设置ATK-MC5640模块镜像/翻转 */
uint8_t atk_mc5640_set_test_pattern(atk_mc5640_test_pattern_t pattern);                                     /* 设置ATK-MC5640模块测试图案 */
uint8_t atk_mc5640_set_output_format(atk_mc5640_output_format_t format);                                    /* 设置ATK-MC5640模块输出图像格式 */
uint8_t atk_mc5640_set_isp_input_window(uint16_t x, uint16_t y, uint16_t width, uint16_t height);           /* 设置ATK-MC5640模块ISP输入窗口尺寸 */
uint8_t atk_mc5640_set_pre_scaling_window(uint16_t x_offset, uint16_t y_offset);                            /* 设置ATK-MC5640模块预缩放窗口偏移 */
uint8_t atk_mc5640_set_output_size(uint16_t width, uint16_t height);                                        /* 设置ATK-MC5640模块输出图像尺寸 */
uint8_t atk_mc5640_get_frame(uint32_t dts_addr, atk_mc5640_get_type_t type, void (*before_transfer)(void)); /* 获取ATK-MC5640模块输出的一帧图像数据 */
uint8_t atk_mc5640_read_reg(uint16_t reg);
void atk_mc5640_write_reg(uint16_t reg, uint8_t dat);
void OV5640_Apply_Best_Settings(void);
#endif
