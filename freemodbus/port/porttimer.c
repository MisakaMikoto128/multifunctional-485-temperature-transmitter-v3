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

/* ----------------------- Modbus includes ----------------------------------*/
#include "mb.h"
#include "mbport.h"
#include "HDL_G4_CPU_Time.h"
/* ----------------------- Defines ------------------------------------------*/
void prvvTIMERExpiredISR(void);
static USHORT gusTim1Timeoutus = 0;
#define CPU_US_TIME_CH 1
/* ----------------------- Static variables ---------------------------------*/

/* ----------------------- Start implementation -----------------------------*/
BOOL xMBPortTimersInit(USHORT usTim1Timeout50us)
{
    BOOL bInitialized = TRUE;
    // 每次调用这个方法都会重置一次定时器
    HDL_G4_CPU_Time_Init();
    gusTim1Timeoutus = usTim1Timeout50us * 50;
    // 初始化成功要返回TRUE通知初始化完成
    return bInitialized;
}

void vMBPortTimersEnable(void)
{
    // 每次调用这个方法都会重置一次到达超时中断的时间，这个都是在接收到一个字符的时候调用的
    //  调用这个函数同时意味着是能超时中断
    HDL_G4_CPU_Time_StartHardTimer(CPU_US_TIME_CH, gusTim1Timeoutus, prvvTIMERExpiredISR);
}

void vMBPortTimersDisable(void)
{
    // 调用这个函数意味着关闭定时器中断或者关闭定时器
    HDL_G4_CPU_Time_StopHardTimer(CPU_US_TIME_CH);
}

void vMBPortTimersDelay(USHORT usTimeOutMS)
{
    UNUSED(usTimeOutMS);
}

void prvvTIMERExpiredISR(void)
{
    (void)pxMBPortCBTimerExpired();
}
