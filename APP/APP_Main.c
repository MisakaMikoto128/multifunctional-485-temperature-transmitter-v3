/**
 * @file APP_Main.c
 * @author Liu Yuanlin (liuyuanlins@outlook.com)
 * @brief
 * @version 0.1
 * @date 2024-03-28
 * @last modified 2024-03-28
 *
 * @copyright Copyright (c) 2024 Liu Yuanlin Personal.
 *
 */
#include "APP_Main.h"
#include <stdlib.h>
#include "HDL_G4_CPU_Time.h"
#include "APP_Modbus_Slaver.h"
#include "APP_Ethernet.h"
#include "BFL_Debug.h"

uint16_t *eMBGetRegHoldingBufBase(size_t *usLen);

SysInfo_t sysinfo = {0};

void SysInfoWriteToRegHoldingBuf(const SysInfo_t *pSysInfo)
{
    size_t usLen             = 0;
    uint16_t *pRegHoldingBuf = eMBGetRegHoldingBufBase(&usLen);

    /*
    寄存器地址	数据类型	单位 (缩写)	精度	范围	描述
    0x0000	16位有符号整数	摄氏度 (℃)	0.01 ℃	15-110 ℃	NTC热敏电阻1温度寄存器
    0x0001	16位有符号整数	摄氏度 (℃)	0.01 ℃	15-110 ℃	NTC热敏电阻1温度寄存器
    */
    int16_t int16Var  = 0;
    int16Var          = (int16_t)(pSysInfo->Temp1 * 100);
    pRegHoldingBuf[0] = int16Var;
    int16Var          = (int16_t)(pSysInfo->Temp2 * 100);
    pRegHoldingBuf[1] = int16Var;
    int16Var          = (int16_t)(pSysInfo->Temp3 * 100);
    pRegHoldingBuf[2] = int16Var;
    int16Var          = (int16_t)(pSysInfo->Temp4 * 100);
    pRegHoldingBuf[3] = int16Var;
    int16Var          = (int16_t)(pSysInfo->Temp5 * 100);
    pRegHoldingBuf[4] = int16Var;
    int16Var          = (int16_t)(pSysInfo->TempD1 * 100);
    pRegHoldingBuf[5] = int16Var;
    int16Var          = (int16_t)(pSysInfo->TempD2 * 100);
    pRegHoldingBuf[6] = int16Var;
    int16Var          = (int16_t)(pSysInfo->TempD3 * 100);
    pRegHoldingBuf[7] = int16Var;
    int16Var          = (int16_t)(pSysInfo->TempD4 * 100);
    pRegHoldingBuf[8] = int16Var;
    int16Var          = (int16_t)(pSysInfo->TempD5 * 100);
    pRegHoldingBuf[9] = int16Var;
}

static PeriodREC_t s_tPollTime  = 0;
static PeriodREC_t s_tPollTime2 = 0;
static PeriodREC_t s_tPollTime3 = 0;
#include "CHIP_MIZ043WO1RPGA.h"
#include "log.h"
#include "BFL_LED.h"
#include "SysMeasure.h"
#include "CHIP_MCP4017T_104.h"
#include "CHIP_TCA9548A.h"
#include "CHIP_TMP102.h"
#include "tmp102.h"

extern struct MCP4017T_104_Chip_t g_mcp4017t_104_chip_list[MCP4017T_104_NUM];
extern uint8_t g_mcp4017t_i2c_channel_map[MCP4017T_104_NUM];

#include "socket.h" // Just include one header for WIZCHIP
extern wiz_NetInfo gWIZNETINFO;

void IIC_Test()
{
    CHIP_TCA9548A_SelectChannel(g_mcp4017t_i2c_channel_map[5]);

    uint8_t wiper_set = 0;
    CHIP_MCP4017T_104_GetWiper(&g_mcp4017t_104_chip_list[5], &wiper_set);
    ULOG_INFO("Wiper get before set : %d", wiper_set);

    wiper_set = 45;
    CHIP_MCP4017T_104_SetWiper(&g_mcp4017t_104_chip_list[5], wiper_set);

    uint8_t wiper_get = 0;
    CHIP_MCP4017T_104_GetWiper(&g_mcp4017t_104_chip_list[5], &wiper_get);
    ULOG_INFO("Wiper get after set: %d", wiper_get);

    if (wiper_get != wiper_set) {
        ULOG_ERROR("[Error] Wiper set error");
    } else {
        ULOG_INFO("[OK] Wiper set success");
    }

    CHIP_TCA9548A_SelectChannel(0); // 选择通道0
    float tempture = 0;
    if (CHIP_TMP102_GetTemperature(&tempture)) {
        ULOG_INFO("Temperature: %f", tempture);
    }
}

void APP_Main_Poll()
{
    if (period_query_user(&s_tPollTime, 100)) {
        if (sysinfo.isT1Connected) {
            CHIP_MIZ043WO1RPGA_Printf(0, 6, "%6.2f", sysinfo.Temp1);
        } else {
            CHIP_MIZ043WO1RPGA_Printf(0, 6, " T1 NC");
        }

        if (sysinfo.isT2Connected) {
            CHIP_MIZ043WO1RPGA_Printf(6, 6, "%6.2f", sysinfo.Temp2);
        } else {
            CHIP_MIZ043WO1RPGA_Printf(6, 6, " T2 NC");
        }

        if (sysinfo.isT3Connected) {
            CHIP_MIZ043WO1RPGA_Printf(12, 6, "%6.2f", sysinfo.Temp3);
        } else {
            CHIP_MIZ043WO1RPGA_Printf(12, 6, " T3 NC");
        }

        if (sysinfo.isT4Connected) {
            CHIP_MIZ043WO1RPGA_Printf(18, 6, "%6.2f", sysinfo.Temp4);
        } else {
            CHIP_MIZ043WO1RPGA_Printf(18, 6, " T4 NC");
        }

        if (sysinfo.isT5Connected) {
            CHIP_MIZ043WO1RPGA_Printf(24, 6, "%6.2f", sysinfo.Temp5);
        } else {
            CHIP_MIZ043WO1RPGA_Printf(24, 6, " T5 NC");
        }

        if (sysinfo.isD1Connected) {
            CHIP_MIZ043WO1RPGA_Printf(30, 6, "%6.2f", sysinfo.TempD1);
        } else {
            CHIP_MIZ043WO1RPGA_Printf(30, 6, " D1 NC");
        }

        CHIP_MIZ043WO1RPGA_Printf(36, 27, "%d.%d.%d.%d:%d", gWIZNETINFO.ip[0], gWIZNETINFO.ip[1], gWIZNETINFO.ip[2], gWIZNETINFO.ip[3], 5050);
    }

    if (period_query_user(&s_tPollTime2, 10)) {
        BFL_LED_IncBrightness(LED1, 14);
        // 更新温度到Modbus Slave寄存器
        SysInfoWriteToRegHoldingBuf(&sysinfo);
    }

    if (period_query_user(&s_tPollTime3, 500)) {
        // IIC_Test();
        CHIP_TCA9548A_SelectChannel(0); // 选择通道0
        float tempture = 0;
        if (CHIP_TMP102_GetTemperature(&tempture)) {
            ULOG_INFO("Temperature: %f", tempture);
            sysinfo.TempD1        = tempture;
            sysinfo.isD1Connected = true;
        } else {
            ULOG_ERROR("Temperature read failed");
            sysinfo.isD1Connected = false;
        }
    }
}

void APP_Main_Init()
{
    HDL_G4_CPU_Time_Init();
    ulog_init_user();
    BFL_LED_Init();
    BFL_Debug_Init();
    CHIP_MIZ043WO1RPGA_Init();
    CHIP_TCA9548A_Init();

    for (int i = 0; i < MCP4017T_104_NUM; i++) {
        CHIP_TCA9548A_SelectChannel(g_mcp4017t_i2c_channel_map[i]);
        CHIP_MCP4017T_104_Init(&g_mcp4017t_104_chip_list[i]);
    }

    CHIP_TCA9548A_SelectChannel(0); // 选择通道0
    CHIP_TMP102_Init();

    APP_Ethernet_Init();
    APP_Modbus_Slaver_Init();
    SysMeasureInit();
    asyn_sys_register(APP_Main_Poll);
}
