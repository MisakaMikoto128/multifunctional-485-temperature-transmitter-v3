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
#include <string.h>

uint16_t *eMBGetRegHoldingBufBase(size_t *usLen);

SysInfo_t sysinfo               = {0};
const SysInfo_t sysinfo_default = {
    .MAC  = {0x00, 0x08, 0xdc, 0x16, 0xed, 0x2c},
    .IP   = {192, 168, 1, 12},
    .MASK = {255, 255, 255, 0},
    .GW   = {192, 168, 1, 1},
    .DNS  = {8, 8, 8, 8},
    .PORT = 5050};

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
    int16Var          = pSysInfo->isT1Connected ? (int16_t)(pSysInfo->Temp1 * 100) : -27315;
    pRegHoldingBuf[0] = int16Var;
    int16Var          = pSysInfo->isT2Connected ? (int16_t)(pSysInfo->Temp2 * 100) : -27315;
    pRegHoldingBuf[1] = int16Var;
    int16Var          = pSysInfo->isT3Connected ? (int16_t)(pSysInfo->Temp3 * 100) : -27315;
    pRegHoldingBuf[2] = int16Var;
    int16Var          = pSysInfo->isT4Connected ? (int16_t)(pSysInfo->Temp4 * 100) : -27315;
    pRegHoldingBuf[3] = int16Var;
    int16Var          = pSysInfo->isT5Connected ? (int16_t)(pSysInfo->Temp5 * 100) : -27315;
    pRegHoldingBuf[4] = int16Var;
    int16Var          = pSysInfo->isD1Connected ? (int16_t)(pSysInfo->TempD1 * 100) : -27315;
    pRegHoldingBuf[5] = int16Var;
    int16Var          = pSysInfo->isD2Connected ? (int16_t)(pSysInfo->TempD2 * 100) : -27315;
    pRegHoldingBuf[6] = int16Var;
    int16Var          = pSysInfo->isD3Connected ? (int16_t)(pSysInfo->TempD3 * 100) : -27315;
    pRegHoldingBuf[7] = int16Var;
    int16Var          = pSysInfo->isD4Connected ? (int16_t)(pSysInfo->TempD4 * 100) : -27315;
    pRegHoldingBuf[8] = int16Var;
    int16Var          = pSysInfo->isD5Connected ? (int16_t)(pSysInfo->TempD5 * 100) : -27315;
    pRegHoldingBuf[9] = int16Var;
}

void SysInfoWriteNetworkParaToRegHoldingBuf(const SysInfo_t *pSysInfo)
{
    uint16_t *pRegHoldingBuf = eMBGetRegHoldingBufBase(NULL);
    // 10-17 MAC,IP,GW,MASK,DNS,PORT
    pRegHoldingBuf[10] = (uint16_t)(pSysInfo->MAC[0] << 8 | pSysInfo->MAC[1]);
    pRegHoldingBuf[11] = (uint16_t)(pSysInfo->MAC[2] << 8 | pSysInfo->MAC[3]);
    pRegHoldingBuf[12] = (uint16_t)(pSysInfo->MAC[4] << 8 | pSysInfo->MAC[5]);
    pRegHoldingBuf[13] = (uint16_t)(pSysInfo->IP[0] << 8 | pSysInfo->IP[1]);
    pRegHoldingBuf[14] = (uint16_t)(pSysInfo->IP[2] << 8 | pSysInfo->IP[3]);
    pRegHoldingBuf[15] = (uint16_t)(pSysInfo->GW[0] << 8 | pSysInfo->GW[1]);
    pRegHoldingBuf[16] = (uint16_t)(pSysInfo->GW[2] << 8 | pSysInfo->GW[3]);
    pRegHoldingBuf[17] = (uint16_t)(pSysInfo->MASK[0] << 8 | pSysInfo->MASK[1]);
    pRegHoldingBuf[18] = (uint16_t)(pSysInfo->MASK[2] << 8 | pSysInfo->MASK[3]);
    pRegHoldingBuf[19] = (uint16_t)(pSysInfo->DNS[0] << 8 | pSysInfo->DNS[1]);
    pRegHoldingBuf[20] = (uint16_t)(pSysInfo->DNS[2] << 8 | pSysInfo->DNS[3]);
    pRegHoldingBuf[21] = pSysInfo->PORT;
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

#include "SysMeasure.h"
#include "usbd_cdc_if.h"

extern struct SysMeasureData_t sysmeasure_data;
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

        CHIP_MIZ043WO1RPGA_Printf(36, 27, "%d.%d.%d.%d:%d    ", sysinfo.IP[0], sysinfo.IP[1], sysinfo.IP[2], sysinfo.IP[3], sysinfo.PORT);
    }

    if (period_query_user(&s_tPollTime2, 1)) {
        BFL_LED_IncBrightness(LED1, 2);
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

        ULOG_INFO("------------Device Info------------");
        ULOG_INFO("------------刘沅林设计-Yuanlin Design------------");
        ULOG_INFO("T1: %14s, %.5f℃\tR1: %.5fKΩ\tR1_NTC: %.5fKΩ\tV:%.5fV\t", sysinfo.isT1Connected ? "Connected" : "Not Connected", sysmeasure_data.Temp1, sysmeasure_data.resistor1, sysmeasure_data.resistor1_ntc, sysmeasure_data.sensor1);
        ULOG_INFO("T2: %14s, %.5f℃\tR2: %.5fKΩ\tR2_NTC: %.5fKΩ\tV:%.5fV\t", sysinfo.isT2Connected ? "Connected" : "Not Connected", sysmeasure_data.Temp2, sysmeasure_data.resistor2, sysmeasure_data.resistor2_ntc, sysmeasure_data.sensor2);
        ULOG_INFO("T3: %14s, %.5f℃\tR3: %.5fKΩ\tR3_NTC: %.5fKΩ\tV:%.5fV\t", sysinfo.isT3Connected ? "Connected" : "Not Connected", sysmeasure_data.Temp3, sysmeasure_data.resistor3, sysmeasure_data.resistor3_ntc, sysmeasure_data.sensor3);
        ULOG_INFO("T4: %14s, %.5f℃\tR4: %.5fKΩ\tR4_NTC: %.5fKΩ\tV:%.5fV\t", sysinfo.isT4Connected ? "Connected" : "Not Connected", sysmeasure_data.Temp4, sysmeasure_data.resistor4, sysmeasure_data.resistor4_ntc, sysmeasure_data.sensor4);
        ULOG_INFO("T5: %14s, %.5f℃\tR5: %.5fKΩ\tR5_NTC: %.5fKΩ\tV:%.5fV\t", sysinfo.isT5Connected ? "Connected" : "Not Connected", sysmeasure_data.Temp5, sysmeasure_data.resistor5, sysmeasure_data.resistor5_ntc, sysmeasure_data.sensor5);
        ULOG_INFO("D1: %14s, %.5f℃\t", sysinfo.isD1Connected ? "Connected" : "Not Connected", sysmeasure_data.TempD1);
        ULOG_INFO("REF: %.5f V\t", sysmeasure_data.ref);
        ULOG_INFO("IP: %d.%d.%d.%d\tPORT: %d", sysinfo.IP[0], sysinfo.IP[1], sysinfo.IP[2], sysinfo.IP[3], sysinfo.PORT);
        ULOG_INFO("MAC: %02X:%02X:%02X:%02X:%02X:%02X", sysinfo.MAC[0], sysinfo.MAC[1], sysinfo.MAC[2], sysinfo.MAC[3], sysinfo.MAC[4], sysinfo.MAC[5]);
        ULOG_INFO("GW: %d.%d.%d.%d\tMASK: %d.%d.%d.%d\tDNS: %d.%d.%d.%d",
                  sysinfo.GW[0], sysinfo.GW[1], sysinfo.GW[2], sysinfo.GW[3],
                  sysinfo.MASK[0], sysinfo.MASK[1], sysinfo.MASK[2], sysinfo.MASK[3],
                  sysinfo.DNS[0], sysinfo.DNS[1], sysinfo.DNS[2], sysinfo.DNS[3]);
    }
}

void DestroyApp()
{
    ULOG_INFO("DestroyApp");
    HAL_FLASH_Unlock();
    // 擦除最后一个扇区
    FLASH_EraseInitTypeDef EraseInitStruct;
    EraseInitStruct.TypeErase = FLASH_TYPEERASE_PAGES;
    EraseInitStruct.Banks     = FLASH_BANK_1;
    EraseInitStruct.Page      = 0;
    EraseInitStruct.NbPages   = 1;
    uint32_t PageError;
    HAL_FLASHEx_Erase(&EraseInitStruct, &PageError);
    HAL_FLASH_Lock();
    ULOG_INFO("DestroyApp success");
    ULOG_INFO("Rebooting...");
    HAL_NVIC_SystemReset();
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

    sysinfo = sysinfo_default;
    LoadSysInfoFromFlash(&sysinfo);
    SysInfoWriteToRegHoldingBuf(&sysinfo);
    SysInfoWriteNetworkParaToRegHoldingBuf(&sysinfo);

    APP_Ethernet_Init();
    APP_Modbus_Slaver_Init();
    SysMeasureInit();
    asyn_sys_register(APP_Main_Poll);

    const char *str = "__   __                 _ _         ____            _             \n"
                      "\\ \\ / /   _  __ _ _ __ | (_)_ __   |  _ \\  ___  ___(_) __ _ _ __  \n"
                      " \\ V / | | |/ _` | '_ \\| | | '_ \\  | | | |/ _ \\/ __| |/ _` | '_ \\ \n"
                      "  | || |_| | (_| | | | | | | | | | | |_| |  __/\\__ \\ | (_| | | | |\n"
                      "  |_| \\__,_|\\__,_|_| |_|_|_|_| |_| |____/ \\___||___/_|\\__, |_| |_|\n"
                      "                                                     |___/        \n";

#define USB_CDC_BLOCK_SIZE 64
    for (int i = 0; i < strlen(str); i += USB_CDC_BLOCK_SIZE) {
        Debug_Printf(str + i, USB_CDC_BLOCK_SIZE);
        LL_mDelay(10);
    }
}

#include "stm32g4xx_hal_flash.h"

void SaveSysInfo()
{
    // 从Modbus寄存器中读取网络参数
    uint16_t *pRegHoldingBuf = eMBGetRegHoldingBufBase(NULL);
    sysinfo.MAC[0]           = (uint8_t)(pRegHoldingBuf[10] >> 8);
    sysinfo.MAC[1]           = (uint8_t)(pRegHoldingBuf[10]);
    sysinfo.MAC[2]           = (uint8_t)(pRegHoldingBuf[11] >> 8);
    sysinfo.MAC[3]           = (uint8_t)(pRegHoldingBuf[11]);
    sysinfo.MAC[4]           = (uint8_t)(pRegHoldingBuf[12] >> 8);
    sysinfo.MAC[5]           = (uint8_t)(pRegHoldingBuf[12]);
    sysinfo.IP[0]            = (uint8_t)(pRegHoldingBuf[13] >> 8);
    sysinfo.IP[1]            = (uint8_t)(pRegHoldingBuf[13]);
    sysinfo.IP[2]            = (uint8_t)(pRegHoldingBuf[14] >> 8);
    sysinfo.IP[3]            = (uint8_t)(pRegHoldingBuf[14]);
    sysinfo.GW[0]            = (uint8_t)(pRegHoldingBuf[15] >> 8);
    sysinfo.GW[1]            = (uint8_t)(pRegHoldingBuf[15]);
    sysinfo.GW[2]            = (uint8_t)(pRegHoldingBuf[16] >> 8);
    sysinfo.GW[3]            = (uint8_t)(pRegHoldingBuf[16]);
    sysinfo.MASK[0]          = (uint8_t)(pRegHoldingBuf[17] >> 8);
    sysinfo.MASK[1]          = (uint8_t)(pRegHoldingBuf[17]);
    sysinfo.MASK[2]          = (uint8_t)(pRegHoldingBuf[18] >> 8);
    sysinfo.MASK[3]          = (uint8_t)(pRegHoldingBuf[18]);
    sysinfo.DNS[0]           = (uint8_t)(pRegHoldingBuf[19] >> 8);
    sysinfo.DNS[1]           = (uint8_t)(pRegHoldingBuf[19]);
    sysinfo.DNS[2]           = (uint8_t)(pRegHoldingBuf[20] >> 8);
    sysinfo.DNS[3]           = (uint8_t)(pRegHoldingBuf[20]);
    sysinfo.PORT             = pRegHoldingBuf[21];

    SaveSysInfoToFlash(&sysinfo);

    if (pRegHoldingBuf[28] == 0x55AA) {
        pRegHoldingBuf[28] = 0;
        // 重启
        HAL_NVIC_SystemReset();
    }
}

void SaveSysInfoToFlash(const SysInfo_t *pSysInfo)
{
    // 获取最后一个扇区的首地址
    uint32_t FLASH_SECTOR_SIZE = FLASH_PAGE_SIZE;
    uint32_t flash_addr        = FLASH_BASE + FLASH_SIZE - FLASH_SECTOR_SIZE;

    HAL_FLASH_Unlock();

    // 擦除最后一个扇区
    FLASH_EraseInitTypeDef EraseInitStruct;
    EraseInitStruct.TypeErase = FLASH_TYPEERASE_PAGES;
    EraseInitStruct.Banks     = FLASH_BANK_1;
    EraseInitStruct.Page      = (flash_addr - FLASH_BASE) / FLASH_SECTOR_SIZE;
    EraseInitStruct.NbPages   = 1;
    uint32_t PageError;
    HAL_FLASHEx_Erase(&EraseInitStruct, &PageError);

    // 将SysInfo_t对象中的网络参数写入到FLASH中
    uint64_t dataArr[8] = {0};
    dataArr[0]          = 0x55AA55AA55AA55AAULL;

    // 获取参数字节数
    uint8_t *pBuf = (uint8_t *)&dataArr[1];
    //
    size_t networdParaSize = sizeof(SysInfo_t) - offsetof(SysInfo_t, MAC);
    for (int i = 0; i < networdParaSize; i++) {
        pBuf[i] = ((uint8_t *)pSysInfo->MAC)[i];
    }
    for (int i = 0; i < sizeof(dataArr) / sizeof(dataArr[0]); i++) {
        HAL_FLASH_Program(FLASH_TYPEPROGRAM_DOUBLEWORD, flash_addr + i * 8, dataArr[i]);
    }

    HAL_FLASH_Lock();
}

void LoadSysInfoFromFlash(SysInfo_t *pSysInfo)
{
    // 获取最后一个扇区的首地址
    uint32_t FLASH_SECTOR_SIZE = FLASH_PAGE_SIZE;
    uint32_t flash_addr        = FLASH_BASE + FLASH_SIZE - FLASH_SECTOR_SIZE;

    uint64_t dataArr[8] = {0};
    for (int i = 0; i < 7; i++) {
        dataArr[i] = *(uint64_t *)(flash_addr + i * 8);
    }

    if (dataArr[0] != 0x55AA55AA55AA55AAULL) {
        ULOG_ERROR("SysInfo data error");
        SaveSysInfoToFlash(&sysinfo_default);
        return;
    }

    // 将FLASH中的网络参数写入到SysInfo_t对象中
    uint8_t *pBuf          = (uint8_t *)&dataArr[1];
    size_t networdParaSize = sizeof(SysInfo_t) - offsetof(SysInfo_t, MAC);
    for (int i = 0; i < networdParaSize; i++) {
        ((uint8_t *)pSysInfo->MAC)[i] = pBuf[i];
    }
}