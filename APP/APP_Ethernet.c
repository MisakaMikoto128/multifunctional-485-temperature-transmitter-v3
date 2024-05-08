/**
 * @file APP_Ethernet.c
 * @author Liu Yuanlin (liuyuanlins@outlook.com)
 * @brief
 * @version 0.1
 * @date 2024-03-28
 * @last modified 2024-03-28
 *
 * @copyright Copyright (c) 2024 Liu Yuanlin Personal.
 *
 */
#include "APP_Ethernet.h"
#include "APP_Main.h"
#include "HDL_G4_CPU_Time.h"
#include "log.h"
#include "mb.h"

#include "spi.h"
#include "socket.h" // Just include one header for WIZCHIP
#include <stdio.h>
#include <string.h>

#define SOCK_UDPS       0
#define SOCK_MODBUS_TCP 1
#define SOCK_TEMP_UDPS  2

#define DATA_BUF_SIZE   2048
uint8_t gDATABUF[DATA_BUF_SIZE];
// Default Network Configuration
wiz_NetInfo gWIZNETINFO = {.mac  = {0x00, 0x08, 0xdc, 0x16, 0xed, 0x2c},
                           .ip   = {192, 168, 1, 19},
                           .sn   = {255, 255, 255, 0},
                           .gw   = {192, 168, 1, 1},
                           .dns  = {8, 8, 8, 8},
                           .dhcp = NETINFO_STATIC};

void CHIP_W5500_Platform_Init(void);                 // initialize the dependent host peripheral
void CHIP_W5500_NetworkParameterConfiguration(void); // Initialize Network information and display it
void CHIP_W5500_Init(void);

#define W5500_SPI_HANDLE hspi3
void SPI_WriteByte(uint8_t TxData)
{
    HAL_StatusTypeDef ret = HAL_OK;
    ret                   = HAL_SPI_Transmit(&W5500_SPI_HANDLE, &TxData, 1, 10);
    if (ret != HAL_OK) {
        ULOG_INFO("%s error\n", __func__);
    }
}

uint8_t SPI_ReadByte(void)
{
    HAL_StatusTypeDef ret = HAL_OK;
    uint8_t value;

    ret = HAL_SPI_Receive(&W5500_SPI_HANDLE, &value, 1, 10);

    if (ret != HAL_OK) {
        value = 0;
        ULOG_INFO("%s error\n", __func__);
    }
    return value;
}

/**
 * @brief  进入临界区
 * @retval None
 */
void SPI_CrisEnter(void)
{
    __set_PRIMASK(1);
}
/**
 * @brief  退出临界区
 * @retval None
 */
void SPI_CrisExit(void)
{
    __set_PRIMASK(0);
}

/**
 * @brief  片选信号输出低电平
 * @retval None
 */
void SPI_CS_Select(void)
{
    HAL_GPIO_WritePin(SPI3_NSS_GPIO_Port, SPI3_NSS_Pin, GPIO_PIN_RESET);
}

/**
 * @brief  片选信号输出高电平
 * @retval None
 */
void SPI_CS_Deselect(void)
{
    HAL_GPIO_WritePin(SPI3_NSS_GPIO_Port, SPI3_NSS_Pin, GPIO_PIN_SET);
}

void CHIP_W5500_Platform_Init(void)
{
    ; // Have been initialized by CubeMX
}

#define W5500_USING_RST_PIN 1
#define W5500_USING_INT_PIN 1

#if W5500_USING_RST_PIN
#define W5500_RST_PIN_SET()   HAL_GPIO_WritePin(RST_L_GPIO_Port, RST_L_Pin, GPIO_PIN_SET)
#define W5500_RST_PIN_RESET() HAL_GPIO_WritePin(RST_L_GPIO_Port, RST_L_Pin, GPIO_PIN_RESET)
#endif

#if W5500_USING_INT_PIN
#define W5500_INT_PIN_READ()  HAL_GPIO_ReadPin(INT_L_GPIO_Port, INT_L_Pin)
#define W5500_INT_PIN_SET()   HAL_GPIO_WritePin(INT_L_GPIO_Port, INT_L_Pin, GPIO_PIN_SET)
#define W5500_INT_PIN_RESET() HAL_GPIO_WritePin(INT_L_GPIO_Port, INT_L_Pin, GPIO_PIN_RESET)
#endif

#define USE_AUTONEGO

/**
 * @brief W5500复位
 *
 */
void CHIP_W5500_Rest(void)
{
    W5500_RST_PIN_RESET();
    HAL_Delay(50);
    W5500_RST_PIN_SET();
    HAL_Delay(50);
}

/**
 * @brief   set phy config if autonego is disable
 * @param   none
 * @return  none
 */
static void w5500_phy_init(void)
{
#ifdef USE_AUTONEGO
    // no thing to do
#else
    wiz_PhyConf conf;

    conf.by     = PHY_CONFBY_SW;
    conf.mode   = PHY_MODE_MANUAL;
    conf.speed  = PHY_SPEED_100;
    conf.duplex = PHY_DUPLEX_FULL;

    wizphy_setphyconf(&conf);
#endif
}

/**
 * @brief 初始化芯片参数
 *
 */
void CHIP_W5500_ChipParametersConfiguration()
{
    uint8_t tmp;
    uint8_t memsize[2][8] = {{2, 2, 2, 2, 2, 2, 2, 2}, {2, 2, 2, 2, 2, 2, 2, 2}};

    /* WIZCHIP SOCKET Buffer initialize */
    if (ctlwizchip(CW_INIT_WIZCHIP, (void *)memsize) == -1) {
        ULOG_INFO("WIZCHIP Initialized fail.\r\n");
        // Error_Handler();
    }

    w5500_phy_init();

    /* PHY link status check */
    /*
    CW_GET_PHYLINK这个方法可以判断是否连接上了网线。
    do {
        if (ctlwizchip(CW_GET_PHYLINK, (void *)&tmp) == -1) {
            ULOG_INFO("Unknown PHY Link stauts.\r\n");
        }
    } while (tmp == PHY_LINK_OFF);
    */
    if (ctlwizchip(CW_GET_PHYLINK, (void *)&tmp) == -1) {
        ULOG_INFO("Unknown PHY Link stauts.\r\n");
    }
}

void CHIP_W5500_NetworkParameterConfiguration()
{
    uint8_t tmpstr[7] = {0};
    wiz_NetInfo conf;
    // 配置网络地址
    ctlnetwork(CN_SET_NETINFO, (void *)&gWIZNETINFO);
    // 回读
    ctlnetwork(CN_GET_NETINFO, (void *)&conf);

    if (memcmp(&conf, &gWIZNETINFO, sizeof(wiz_NetInfo)) == 0) {
        // 配置成功
        ULOG_INFO("W5500 Network Config Success!\r\n");
    } else {
        // 配置失败
        ULOG_INFO("W5500 Network Config Fail!\r\n");
    }

    // Display Network Information
    ctlwizchip(CW_GET_ID, (void *)tmpstr);
    ULOG_INFO("\r\n=== %s NET CONF ===\r\n", (char *)tmpstr);
    ULOG_INFO("MAC: %02X:%02X:%02X:%02X:%02X:%02X\r\n", conf.mac[0], conf.mac[1], conf.mac[2],
              conf.mac[3], conf.mac[4], conf.mac[5]);
    ULOG_INFO("SIP: %d.%d.%d.%d\r\n", conf.ip[0], conf.ip[1], conf.ip[2], conf.ip[3]);
    ULOG_INFO("GAR: %d.%d.%d.%d\r\n", conf.gw[0], conf.gw[1], conf.gw[2], conf.gw[3]);
    ULOG_INFO("SUB: %d.%d.%d.%d\r\n", conf.sn[0], conf.sn[1], conf.sn[2], conf.sn[3]);
    ULOG_INFO("DNS: %d.%d.%d.%d\r\n", conf.dns[0], conf.dns[1], conf.dns[2], conf.dns[3]);
    ULOG_INFO("======================\r\n");
}

void CHIP_W5500_Init(void)
{
    // Host dependent peripheral initialized
    CHIP_W5500_Platform_Init();
    // W5500 reset
    CHIP_W5500_Rest();
    // First of all, Should register SPI callback functions implemented by user for accessing WIZCHIP
    /* Critical section callback */
    reg_wizchip_cris_cbfunc(SPI_CrisEnter, SPI_CrisExit); // 注册临界区函数
                                                          /* Chip selection call back */
#if _WIZCHIP_IO_MODE_ == _WIZCHIP_IO_MODE_SPI_VDM_
    reg_wizchip_cs_cbfunc(SPI_CS_Select, SPI_CS_Deselect); // 注册SPI片选信号函数
#elif _WIZCHIP_IO_MODE_ == _WIZCHIP_IO_MODE_SPI_FDM_
    reg_wizchip_cs_cbfunc(SPI_CS_Select, SPI_CS_Deselect); // CS must be tried with LOW.
#else
#if (_WIZCHIP_IO_MODE_ & _WIZCHIP_IO_MODE_SIP_) != _WIZCHIP_IO_MODE_SIP_
#error "Unknown _WIZCHIP_IO_MODE_"
#else
    reg_wizchip_cs_cbfunc(wizchip_select, wizchip_deselect);
#endif
#endif
    /* SPI Read & Write callback function */
    reg_wizchip_spi_cbfunc(SPI_ReadByte, SPI_WriteByte); // 注册读写函数

    CHIP_W5500_ChipParametersConfiguration();
    /* Network initialization */
    CHIP_W5500_NetworkParameterConfiguration();
}

void W5500_App_Poll()
{
    // static PeriodREC_t udp_period_rec = 0;
    switch (getSn_SR(SOCK_UDPS)) // 获取socket0的状态
    {
        case SOCK_UDP: // Socket处于初始化完成(打开)状态
            if (getSn_IR(SOCK_UDPS) & Sn_IR_RECV) {
                setSn_IR(SOCK_UDPS, Sn_IR_RECV); // Sn_IR的RECV位置1
            }
            // 数据回环测试程序：数据从远程上位机发给W5500，W5500接收到数据后再回给远程上位机
            int16_t ret = getSn_RX_RSR(SOCK_UDPS);
            if (ret > 0) {
                uint8_t remoteIP[4];
                uint16_t remotePort;
                memset(gDATABUF, 0, ret + 1);
                // W5500接收来自远程上位机的数据，并通过SPI发送给MCU
                int32_t received = recvfrom(SOCK_UDPS, gDATABUF, sizeof(gDATABUF), remoteIP, &remotePort);
                ULOG_INFO("%s\r\n", gDATABUF);                          // 串口打印接收到的数据
                sendto(SOCK_UDPS, gDATABUF, ret, remoteIP, remotePort); // 接收到数据后再回给远程上位机，完成数据回环
            }
            break;
        case SOCK_CLOSED:                             // Socket处于关闭状态
            socket(SOCK_UDPS, Sn_MR_UDP, 5000, 0x00); // 打开Socket0，并配置为UDP模式，打开一个本地端口
            break;
    }
}

extern SysInfo_t sysinfo;
void DestroyApp();

void W5500_UDP_Temp_App_Poll()
{
    // static PeriodREC_t udp_period_rec = 0;
    switch (getSn_SR(SOCK_TEMP_UDPS)) // 获取socket0的状态
    {
        case SOCK_UDP: // Socket处于初始化完成(打开)状态
            if (getSn_IR(SOCK_TEMP_UDPS) & Sn_IR_RECV) {
                setSn_IR(SOCK_TEMP_UDPS, Sn_IR_RECV); // Sn_IR的RECV位置1
            }
            // 数据回环测试程序：数据从远程上位机发给W5500，W5500接收到数据后再回给远程上位机
            int16_t ret = getSn_RX_RSR(SOCK_TEMP_UDPS);
            if (ret > 0) {
                uint8_t remoteIP[4];
                uint16_t remotePort;
                memset(gDATABUF, 0, ret + 1);
                // W5500接收来自远程上位机的数据，并通过SPI发送给MCU
                int32_t received = recvfrom(SOCK_TEMP_UDPS, gDATABUF, sizeof(gDATABUF), remoteIP, &remotePort);
                ULOG_INFO("%s\r\n", gDATABUF); // 串口打印接收到的数据

                uint8_t *pData = gDATABUF;
                // 55 AA CH CNNNECTED T1_INT16_H T1_INT16_L SUM
                uint8_t response[8]  = {0};
                uint16_t responseLen = 7;
                uint16_t requestLen  = 4;
                uint8_t sum          = 0;
                int16_t int16Var     = 0;
                for (size_t i = 0; i < 6; i++) {
                    sum += pData[i];
                }

                if (received >= requestLen && pData[0] == 0x55 && pData[1] == 0xAA && pData[2] == 0x01) {

                    uint8_t channel = pData[3];
                    switch (channel) {
                        case 0:
                            response[2] = 0x00;
                            int16Var    = (int16_t)(sysinfo.Temp1 * 100);
                            response[3] = sysinfo.isT1Connected;
                            response[4] = int16Var >> 8;
                            response[5] = int16Var & 0xFF;
                            break;
                        case 1:
                            response[2] = 0x01;
                            int16Var    = (int16_t)(sysinfo.Temp2 * 100);
                            response[3] = sysinfo.isT2Connected;
                            response[4] = int16Var >> 8;
                            response[5] = int16Var & 0xFF;
                            break;
                        case 2:
                            response[2] = 0x02;
                            int16Var    = (int16_t)(sysinfo.Temp3 * 100);
                            response[3] = sysinfo.isT3Connected;
                            response[4] = int16Var >> 8;
                            response[5] = int16Var & 0xFF;
                            break;
                        case 3:
                            response[2] = 0x03;
                            int16Var    = (int16_t)(sysinfo.Temp4 * 100);
                            response[3] = sysinfo.isT4Connected;
                            response[4] = int16Var >> 8;
                            response[5] = int16Var & 0xFF;
                            break;
                        case 4:
                            response[2] = 0x04;
                            int16Var    = (int16_t)(sysinfo.Temp5 * 100);
                            response[3] = sysinfo.isT5Connected;
                            response[4] = int16Var >> 8;
                            response[5] = int16Var & 0xFF;
                            break;
                        case 5:
                            response[2] = 0x05;
                            int16Var    = (int16_t)(sysinfo.TempD1 * 100);
                            response[3] = sysinfo.isD1Connected;
                            response[4] = int16Var >> 8;
                            response[5] = int16Var & 0xFF;
                            break;
                        case 6:
                            response[2] = 0x06;
                            int16Var    = (int16_t)(sysinfo.TempD2 * 100);
                            response[3] = sysinfo.isD2Connected;
                            response[4] = int16Var >> 8;
                            response[5] = int16Var & 0xFF;
                            break;
                        case 7:
                            response[2] = 0x07;
                            int16Var    = (int16_t)(sysinfo.TempD3 * 100);
                            response[3] = sysinfo.isD3Connected;
                            response[4] = int16Var >> 8;
                            response[5] = int16Var & 0xFF;
                            break;
                        case 8:
                            response[2] = 0x08;
                            int16Var    = (int16_t)(sysinfo.TempD4 * 100);
                            response[3] = sysinfo.isD4Connected;
                            response[4] = int16Var >> 8;
                            response[5] = int16Var & 0xFF;
                            break;
                        case 9:
                            response[2] = 0x09;
                            int16Var    = (int16_t)(sysinfo.TempD5 * 100);
                            response[3] = sysinfo.isD5Connected;
                            response[4] = int16Var >> 8;
                            response[5] = int16Var & 0xFF;
                            break;
                        default:
                            break;
                    }

                    response[0] = 0x55;
                    response[1] = 0xAA;
                    for (int i = 0; i < 6; i++) {
                        sum += response[i];
                    }
                    response[6] = sum;

                    sendto(SOCK_TEMP_UDPS, response, responseLen, remoteIP, remotePort); // 接收到数据后再回给远程上位机，完成数据回环
                }
            }
            break;
        case SOCK_CLOSED:                                          // Socket处于关闭状态
            socket(SOCK_TEMP_UDPS, Sn_MR_UDP, sysinfo.PORT, 0x00); // 打开Socket0，并配置为UDP模式，打开一个本地端口
            break;
    }
}

#define MBTCP_PORT 502
void modbus_tcps();

#include "HDL_G4_CPU_Time.h"
#include "asyn_sys.h"

static void APP_Ethernet_Poll()
{
    static PeriodREC_t s_tPollTime = 0;
    if (period_query_user(&s_tPollTime, 50)) {
        W5500_App_Poll();
        W5500_UDP_Temp_App_Poll();
    }
}

void APP_Ethernet_Init()
{
    memcpy(gWIZNETINFO.mac, sysinfo.MAC, sizeof(sysinfo.MAC));
    memcpy(gWIZNETINFO.ip, sysinfo.IP, sizeof(sysinfo.IP));
    memcpy(gWIZNETINFO.gw, sysinfo.GW, sizeof(sysinfo.GW));
    memcpy(gWIZNETINFO.sn, sysinfo.MASK, sizeof(sysinfo.MASK));
    memcpy(gWIZNETINFO.dns, sysinfo.DNS, sizeof(sysinfo.DNS));

    CHIP_W5500_Init();
    asyn_sys_register(APP_Ethernet_Poll);
}