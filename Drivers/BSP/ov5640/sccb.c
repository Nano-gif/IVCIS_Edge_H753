#include "sccb.h"

/*
 * 微秒级延时
 * 针对 STM32H7 480MHz，使用超大循环确保 I2C 波形极宽
 */
static void SCCB_Delay(void)
{
    volatile uint32_t i = 8000; // 再次加大，确保万无一失
    while(i--);
}

/* 设置 SDA 为输入模式 */
static void SCCB_SDA_IN(void)
{
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    GPIO_InitStruct.Pin = SCCB_SDA_Pin;
    GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
    GPIO_InitStruct.Pull = GPIO_PULLUP;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
    HAL_GPIO_Init(SCCB_SDA_GPIO_Port, &GPIO_InitStruct);
}

/* 设置 SDA 为输出模式 */
static void SCCB_SDA_OUT(void)
{
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    GPIO_InitStruct.Pin = SCCB_SDA_Pin;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_PULLUP;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
    HAL_GPIO_Init(SCCB_SDA_GPIO_Port, &GPIO_InitStruct);
}

/* 初始化 GPIO */
void SCCB_Init(void)
{
    SCCB_SDA_OUT();
    SCCB_SCL_H();
    SCCB_SDA_H();
    SCCB_Delay();
}

/* 起始信号 */
void SCCB_Start(void)
{
    SCCB_SDA_H();
    SCCB_SCL_H();
    SCCB_Delay();
    SCCB_SDA_L();
    SCCB_Delay();
    SCCB_SCL_L();
}

/* 停止信号 */
void SCCB_Stop(void)
{
    SCCB_SDA_L();
    SCCB_Delay();
    SCCB_SCL_H();
    SCCB_Delay();
    SCCB_SDA_H();
    SCCB_Delay();
}

/* 发送 NACK */
void SCCB_No_Ack(void)
{
    SCCB_SDA_H();
    SCCB_Delay();
    SCCB_SCL_H();
    SCCB_Delay();
    SCCB_SCL_L();
    SCCB_Delay();
}

/* 写入一个字节 */
void SCCB_WriteByte(uint8_t data)
{
    uint8_t i;
    for (i = 0; i < 8; i++)
    {
        if (data & 0x80) SCCB_SDA_H();
        else             SCCB_SDA_L();
        data <<= 1;
        SCCB_Delay();
        SCCB_SCL_H();
        SCCB_Delay();
        SCCB_SCL_L();
    }
    SCCB_No_Ack();
}

/* 读取一个字节 */
uint8_t SCCB_ReadByte(void)
{
    uint8_t i, data = 0;
    SCCB_SDA_IN();
    for (i = 0; i < 8; i++)
    {
        SCCB_Delay();
        SCCB_SCL_H();
        data <<= 1;
        if (SCCB_READ_SDA()) data++;
        SCCB_Delay();
        SCCB_SCL_L();
    }
    SCCB_SDA_OUT();
    return data;
}
