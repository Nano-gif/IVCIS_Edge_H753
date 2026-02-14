/**
 ****************************************************************************************************
 * @file        demo.c
 * @author      正点原子团队(ALIENTEK)
 * @version     V1.0
 * @date        2022-06-21
 * @brief       ATK-MC5640模块测试实验（DCMI）
 * @license     Copyright (c) 2020-2032, 广州市星翼电子科技有限公司
 ****************************************************************************************************
 * @attention
 *
 * 实验平台:正点原子 阿波罗 H743开发板
 * 在线视频:www.yuanzige.com
 * 技术论坛:www.openedv.com
 * 公司网址:www.alientek.com
 * 购买地址:openedv.taobao.com
 *
 ****************************************************************************************************
 */

#include "demo.h"
#include "./BSP/ATK_MC5640/atk_mc5640.h"
#include "./SYSTEM/usart/usart.h"
#include "./SYSTEM/delay/delay.h"
#include "./BSP/lcd/lcd.h"
#include "./BSP/led/led.h"

/**
 * @brief       复位LCD的写入位置至坐标为(0, 0)的像素点
 * @param       无
 * @retval      无
 */
static void demo_reset_lcd(void)
{
    lcd_set_cursor(0, 0);
    lcd_write_ram_prepare();
}

/**
 * @brief       例程演示入口函数
 * @param       无
 * @retval      无
 */
void demo_run(void)
{
    uint8_t ret;
    uint16_t output_height;
    
    ret  = atk_mc5640_init();                                               /* 初始化ATK-MC5640模块 */
    ret += atk_mc5640_set_output_format(ATK_MC5640_OUTPUT_FORMAT_RGB565);   /* 设置ATK-MC5640输出RGB565图像数据 */
    ret += atk_mc5640_auto_focus_init();                                    /* 初始化ATK-MC5640模块自动对焦 */
    ret += atk_mc5640_auto_focus_continuance();                             /* ATK-MC5640模块持续自动对焦 */
    ret += atk_mc5640_set_light_mode(ATK_MC5640_LIGHT_MODE_ADVANCED_AWB);   /* 设置ATK-MC5640模块灯光模式 */
    ret += atk_mc5640_set_color_saturation(ATK_MC5640_COLOR_SATURATION_4);  /* 设置ATK-MC5640模块色彩饱度 */
    ret += atk_mc5640_set_brightness(ATK_MC5640_BRIGHTNESS_4);              /* 设置ATK-MC5640模块亮度 */
    ret += atk_mc5640_set_contrast(ATK_MC5640_CONTRAST_4);                  /* 设置ATK-MC5640模块对比度 */
    ret += atk_mc5640_set_hue(ATK_MC5640_HUE_6);                            /* 设置ATK-MC5640模块色相 */
    ret += atk_mc5640_set_special_effect(ATK_MC5640_SPECIAL_EFFECT_NORMAL); /* 设置ATK-MC5640模块特殊效果 */
    ret += atk_mc5640_set_exposure_level(ATK_MC5640_EXPOSURE_LEVEL_5);      /* 设置ATK-MC5640模块曝光度 */
    ret += atk_mc5640_set_sharpness_level(ATK_MC5640_SHARPNESS_OFF);        /* 设置ATK-MC5640模块锐度 */
    ret += atk_mc5640_set_mirror_flip(ATK_MC5640_MIRROR_FLIP_1);            /* 设置ATK-MC5640模块镜像/翻转 */
    ret += atk_mc5640_set_test_pattern(ATK_MC5640_TEST_PATTERN_OFF);        /* 设置ATK-MC5640模块测试图案 */
    if (lcddev.width == 480)
    {
        output_height = 640;
    }
    else
    {
        output_height = lcddev.height;
    }
    ret += atk_mc5640_set_output_size(lcddev.width, output_height);         /* 设置ATK-MC5640模块输出图像尺寸 */
    if (ret != 0)
    {
        printf("ATK-MC5640 init failed!\r\n");
        while (1)
        {
            LED0_TOGGLE();
            delay_ms(200);
        }
    }
    
    while (1)
    {
        /* 将获取到的图像数据，显示至LCD */
        atk_mc5640_get_frame((uint32_t)&LCD->LCD_RAM, ATK_MC5640_GET_TYPE_DTS_16B_NOINC, demo_reset_lcd);
    }
}
