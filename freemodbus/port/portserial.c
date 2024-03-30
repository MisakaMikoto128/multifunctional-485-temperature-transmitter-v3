/*
 * FreeModbus Libary: MSP430 Port
 * Copyright (C) 2006 Christian Walter <wolti@sil.at>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 *
 * File: $Id$
 */

/* ----------------------- Platform includes --------------------------------*/
#include "port.h"
#include "main.h"
#include "HDL_G4_Uart.h"
/* ----------------------- Modbus includes ----------------------------------*/
#include "mb.h"
#include "mbport.h"

/* ----------------------- Defines ------------------------------------------*/
void serialReceiveOneByteISR(void);

void serialSentOneByteISR(void);

static void RS485_ReleaseBus()
{
    LL_GPIO_ResetOutputPin(RS485_CON1_GPIO_Port, RS485_CON1_Pin);
}

static void RS485_TakeBus()
{
    LL_GPIO_SetOutputPin(RS485_CON1_GPIO_Port, RS485_CON1_Pin);
    // LL_mDelay(1 - 1);
}

/* ----------------------- Static variables ---------------------------------*/

/* ----------------------- Start implementation -----------------------------*/
void vMBPortSerialEnable(BOOL xRxEnable, BOOL xTxEnable)
{
    if (xTxEnable) {
        RS485_TakeBus();
    } else {
        RS485_ReleaseBus();
    }

    if (xRxEnable) {
        LL_USART_EnableIT_RXNE(USART3);
    } else {
        LL_USART_DisableIT_RXNE(USART3);
    }

    if (xTxEnable) {
        RS485_TakeBus();
        LL_USART_EnableIT_TC(USART3); // 使能发送完成中断
    } else {
        LL_USART_DisableIT_TC(USART3); // 禁能发送完成中断
    }
}

BOOL xMBPortSerialInit(UCHAR ucPort, ULONG ulBaudRate, UCHAR ucDataBits, eMBParity eParity)
{
    BOOL bInitialized = TRUE;
    /**
     * ucPort 用于区分不同的串口，这里没有使用。
     * ucDataBits 这里再Modbus-RTU下一定是8，所以不使用。这个可以在eMBRTUInit查看。
     * eParity Modbus-RTU要求一帧是11位，所以如果有奇偶校验，那么就是1位停止位，否侧使用2位停止位。
     */
    UNUSED(ucPort);
    uint32_t stopBit = LL_USART_STOPBITS_1;
    uint32_t parity  = LL_USART_PARITY_NONE;

    stopBit = eParity == MB_PAR_NONE ? LL_USART_STOPBITS_2 : LL_USART_STOPBITS_1;
    switch (eParity) {
        case MB_PAR_NONE:
            parity = LL_USART_PARITY_NONE;
            break;
        case MB_PAR_ODD:
            parity = LL_USART_PARITY_ODD;
            break;
        case MB_PAR_EVEN:
            parity = LL_USART_PARITY_EVEN;
            break;
        default:
            break;
    }
    Uart_Init(COM3, ulBaudRate, LL_USART_DATAWIDTH_8B, stopBit, parity);

    // 因为我使用的这个库已经自己实现了接收完成中断，所以这里没有单独列出中断函数来,按照文章说明的意思来即可
    Uart_RegisterReceiveCharNotReadCallback(COM3, serialReceiveOneByteISR);
    Uart_SetWriteOverCallback(COM3, serialSentOneByteISR);

    // 如果初始化失败，应当返回FALSE
    return bInitialized;
}

BOOL xMBPortSerialPutByte(CHAR ucByte)
{
    USART3->TDR = ucByte;
    return TRUE;
}

BOOL xMBPortSerialGetByte(CHAR *pucByte)
{
    *pucByte = USART3->RDR;
    return TRUE;
}

void serialReceiveOneByteISR(void)
{
    pxMBFrameCBByteReceived();
}

void serialSentOneByteISR(void)
{
    pxMBFrameCBTransmitterEmpty();
}

void EnterCriticalSection(void)
{
    __disable_irq();
}

void ExitCriticalSection(void)
{
    __enable_irq();
}
