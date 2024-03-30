/**
 * @file APP_Modbus_Slaver.c
 * @author Liu Yuanlin (liuyuanlins@outlook.com)
 * @brief
 * @version 0.1
 * @date 2024-03-28
 * @last modified 2024-03-28
 *
 * @copyright Copyright (c) 2024 Liu Yuanlin Personal.
 *
 */
#include "APP_Modbus_Slaver.h"
#include "HDL_G4_CPU_Time.h"
#include "log.h"
#include "mb.h"
#include "asyn_sys.h"

/**
 * @brief 
 * 
 * @note This function uses global variables and is not reentrant.
 */
static void APP_Modbus_Slaver_Poll(void)
{
    static PeriodREC_t s_tPollTime = 0;
    if (period_query_user(&s_tPollTime, 1))
    {
        eMBPoll();
    }
}

void APP_Modbus_Slaver_Init()
{
    /*
     * RTU模式 从机地址：0x01 串口：这里不起作用，随便写 波特率：115200 无奇偶校验位
     */
    eMBInit(MB_RTU, 0x01, 0, 115200, MB_PAR_NONE);
    eMBEnable();
    asyn_sys_register(APP_Modbus_Slaver_Poll);
}