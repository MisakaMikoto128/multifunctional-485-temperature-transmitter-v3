/**
 * @file HDL_G4_CPU_Time_test.c
 * @author Liu Yuanlin (liuyuanlins@outlook.com)
 * @brief
 * @version 0.1
 * @date 2022-10-09
 *
 * @copyright Copyright (c) 2022 Liu Yuanlin Personal.
 *
 */

#include "HDL_G4_CPU_Time_test.h"
#include "log.h"
#include "main.h"
/**
 * @brief CPU时钟测试。对比和HALtick的误差。
 *
 */
void HDL_G4_CPU_Time_test()
{
    HDL_G4_CPU_Time_Init();
  

    uint32_t cpu_tick = 0;
    uint32_t cpu_us_tick = 0;
    uint32_t hal_tick = 0;
    while (1)
    {
        hal_tick = HAL_GetTick();
        cpu_tick = HDL_G4_CPU_Time_GetTick();
        cpu_us_tick = HDL_G4_CPU_Time_GetUsTick();
        Debug_Printf("hal_tick = %u,cpu_tick = %u,hal_tick - cpu_tick %d\r\n", hal_tick, cpu_tick, hal_tick - cpu_tick);
        Debug_Printf("cpu_us_tick = %u\r\n", cpu_us_tick);
    }
}

void call1()
{
    static int i = 0;
    Debug_Printf("call 1 = %u\r\n", i);
    HDL_G4_CPU_Time_StartHardTimer(1, 50000U, call1);
}

void call2()
{
    static int i = 0;
    Debug_Printf("call 2 = %u\r\n", i);
    HDL_G4_CPU_Time_StartHardTimer(4, 50000U, call2);
}

/**
 * @brief CPU时钟测试。对比和HALtick的误差。测试TIMx比较器中断。
 *
 */
void HDL_G4_CPU_Time_hard_timer_test()
{
    HDL_G4_CPU_Time_Init();
    
    HDL_G4_CPU_Time_StartHardTimer(1, 50000U, call1);
    HDL_G4_CPU_Time_StartHardTimer(4, 50000U, call2);
    uint32_t cpu_tick = 0;
    uint32_t cpu_us_tick = 0;
    uint32_t hal_tick = 0;
    while (1)
    {
        hal_tick = HAL_GetTick();
        cpu_tick = HDL_G4_CPU_Time_GetTick();
        cpu_us_tick = HDL_G4_CPU_Time_GetUsTick();
        Debug_Printf("hal_tick = %u,cpu_tick = %u,hal_tick - cpu_tick %d\r\n", hal_tick, cpu_tick, hal_tick - cpu_tick);
        Debug_Printf("cpu_us_tick = %u\r\n", cpu_us_tick);
    }
}