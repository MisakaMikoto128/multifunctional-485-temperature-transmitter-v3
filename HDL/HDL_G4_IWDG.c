/**********************************************************************************************************
功	能: 看门狗
作	者: 裘澍天
日	期: 2021.11.01
版	本: V1.0
说	明：
	1、32MHz的LSI时钟
	2、最大溢出时间25s
	2、使用说明：
		在main.c里边USER CODE BEGIN Includes 后添加	#include "HDL_G4_IWDG.h"
		在main函数中USER CODE BEGIN Init 	后添加	HDL_G4_WATCHDOG_Init(溢出时间s);
		在while主循环中尾骨 HDL_G4_WATCHDOG_Feed();
		
		注意：如果是其他系列的需要在HDL_G4_IWDG.h	中修改	#include "stm32g4xx_ll_iwdg.h"
**********************************************************************************************************/
#include "HDL_G4_IWDG.h"

/**********************************************************************************************************
*函 数 名: HDL_G4_WATCHDOG_Init
*功能说明: 看门狗初始化
*形    参: 溢出时间：单位秒
*返 回 值: 无
*说    明：最大溢出时间25s
**********************************************************************************************************/
void HDL_G4_WATCHDOG_Init(uint16_t timeout)
{
	uint16_t x=0;
	x=timeout*32000U/256U;

	LL_IWDG_Enable(IWDG);
	LL_IWDG_EnableWriteAccess(IWDG);
	LL_IWDG_SetPrescaler(IWDG, LL_IWDG_PRESCALER_256);
	LL_IWDG_SetReloadCounter(IWDG, x);
	while (LL_IWDG_IsReady(IWDG) != 1)
	{
	}
	LL_IWDG_ReloadCounter(IWDG);
}

/**********************************************************************************************************
*函 数 名: HDL_G4_WATCHDOG_Feed
*功能说明: 看门狗喂狗
*形    参: 无
*返 回 值: 无
*说    明：无
**********************************************************************************************************/
void HDL_G4_WATCHDOG_Feed(void)
{
	LL_IWDG_ReloadCounter(IWDG);
}
