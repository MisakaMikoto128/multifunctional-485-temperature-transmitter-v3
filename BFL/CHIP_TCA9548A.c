/**
 * @file CHIP_TCA9548A.c
 * @author Liu Yuanlin (liuyuanlins@outlook.com)
 * @brief
 * @version 0.1
 * @date 2024-03-29
 * @last modified 2024-03-29
 *
 * @copyright Copyright (c) 2024 Liu Yuanlin Personal.
 *
 */
#include "CHIP_TCA9548A.h"
#include "i2c.h"
#include "main.h"
#include "log.h"
#define TCA9548A_I2C_HANDLER hi2c1

#ifndef IIC_RESET_Pin
#define IIC_RESET_Pin LL_GPIO_PIN_6
#endif

#ifndef IIC_RESET_GPIO_Port
#define IIC_RESET_GPIO_Port GPIOB
#endif

static int g_current_channel = -1;

void CHIP_TCA9548A_Init()
{
    LL_GPIO_InitTypeDef GPIO_InitStruct = {0};

    LL_AHB2_GRP1_EnableClock(LL_AHB2_GRP1_PERIPH_GPIOB);
    /**/
    LL_GPIO_ResetOutputPin(IIC_RESET_GPIO_Port, IIC_RESET_Pin);

    /**/
    GPIO_InitStruct.Pin        = IIC_RESET_Pin;
    GPIO_InitStruct.Mode       = LL_GPIO_MODE_OUTPUT;
    GPIO_InitStruct.Speed      = LL_GPIO_SPEED_FREQ_LOW;
    GPIO_InitStruct.OutputType = LL_GPIO_OUTPUT_PUSHPULL;
    GPIO_InitStruct.Pull       = LL_GPIO_PULL_NO;
    LL_GPIO_Init(IIC_RESET_GPIO_Port, &GPIO_InitStruct);

    // 使能IIC总线
    LL_GPIO_SetOutputPin(IIC_RESET_GPIO_Port, IIC_RESET_Pin);
}

static const uint8_t g_tca9548a_channel[] = {
    TCA9548A_CHANNEL_0,
    TCA9548A_CHANNEL_1,
    TCA9548A_CHANNEL_2,
    TCA9548A_CHANNEL_3,
    TCA9548A_CHANNEL_4,
    TCA9548A_CHANNEL_5,
    TCA9548A_CHANNEL_6,
    TCA9548A_CHANNEL_7};

/**
 @brief 选择通道打开
 @param channel -[in] 通道号 0-7
 @return 无
*/
void CHIP_TCA9548A_SelectChannel(uint8_t channel)
{
    uint8_t data;
    if (channel > 7) {
        return;
    }

    if(g_current_channel == channel){
        return;
    }

    data                  = g_tca9548a_channel[channel];
    HAL_StatusTypeDef ret = HAL_I2C_Master_Transmit(&hi2c1, (TCA9548A_SLAVE_ADDR << 1) | TCA9548A_WRITE_BIT, &data, 1, 10);
    if(ret != HAL_OK){
        ULOG_ERROR("CHIP_TCA9548A_SelectChannel iic communication failed");
    }else
    {
        g_current_channel = channel;
    }
}