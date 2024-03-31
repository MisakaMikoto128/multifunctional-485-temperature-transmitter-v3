/**
 * @file BFL_Debug.c
 * @author Liu Yuanlin (liuyuanlins@outlook.com)
 * @brief
 * @version 0.1
 * @date 2024-03-30
 * @last modified 2024-03-30
 *
 * @copyright Copyright (c) 2024 Liu Yuanlin Personal.
 *
 */
#include "BFL_Debug.h"
#include "main.h"

void BFL_Debug_Init()
{

    LL_GPIO_InitTypeDef GPIO_InitStruct = {0};

    /**/
    LL_GPIO_ResetOutputPin(Signal_GPIO_Port, Signal_Pin);

    /**/
    GPIO_InitStruct.Pin        = Signal_Pin;
    GPIO_InitStruct.Mode       = LL_GPIO_MODE_OUTPUT;
    GPIO_InitStruct.Speed      = LL_GPIO_SPEED_FREQ_LOW;
    GPIO_InitStruct.OutputType = LL_GPIO_OUTPUT_PUSHPULL;
    GPIO_InitStruct.Pull       = LL_GPIO_PULL_NO;
    LL_GPIO_Init(Signal_GPIO_Port, &GPIO_InitStruct);
}

void BFL_Debug_SignalPinSet()
{
    LL_GPIO_SetOutputPin(Signal_GPIO_Port, Signal_Pin);

}

void BFL_Debug_SignalPinReset()
{
    LL_GPIO_ResetOutputPin(Signal_GPIO_Port, Signal_Pin);
}

void BFL_Debug_SignalPinToggle()
{
    LL_GPIO_TogglePin(Signal_GPIO_Port, Signal_Pin);
}