#include "modbus_host.h"
#include "crc.h"
#include <stdint.h>
#include <stdio.h>
/*
方向引脚:
COM3:
PE12

COM4:
PE10

COM5
PE8
*/
#define RS485_3_DIR_PIN           GPIO_PIN_12
#define RS485_4_DIR_PIN           GPIO_PIN_10
#define RS485_5_DIR_PIN           GPIO_PIN_8
#define RS485_3_DIR_Port          GPIOE
#define RS485_4_DIR_Port          GPIOE
#define RS485_5_DIR_Port          GPIOE
#define RS485_3_DIR_Port_CLK_EN() LL_AHB2_GRP1_EnableClock(LL_AHB2_GRP1_PERIPH_GPIOE)
#define RS485_4_DIR_Port_CLK_EN() LL_AHB2_GRP1_EnableClock(LL_AHB2_GRP1_PERIPH_GPIOE)
#define RS485_5_DIR_Port_CLK_EN() LL_AHB2_GRP1_EnableClock(LL_AHB2_GRP1_PERIPH_GPIOE)

void RS485_SendBuf(ModbusRTUInstance_t *hmodbusRTU, uint8_t *_ucaBuf, uint16_t _usLen);
void modbus_rtu_poll(void);
static void modbus_rtu_host_uart_msg_fatch();
void modbus_rtu_trans_event_inner_handler();
void modbus_rtu_timeout_handler(void);
void modbus_rtu_trans_event_inner_deal(ModbusRTUInstance_t *hmodbusRTU);
uint8_t modbus_rtu_host_have_trans_event(ModbusRTUInstance_t *hmodbusRTU);
uint8_t modbus_rtu_host_query_trans_result(ModbusRTUInstance_t *hmodbusRTU);
static void MODH_RxTimeOut(ModbusRTUInstance_t *hmodbusRTU);

/**
 * @brief 释放hModbusRTU3所在总线。
 *
 */
void releaseModbus3()
{
    LL_GPIO_SetOutputPin(RS485_3_DIR_Port, RS485_3_DIR_PIN);
}

void releaseModbus4()
{
    LL_GPIO_SetOutputPin(RS485_4_DIR_Port, RS485_4_DIR_PIN);
}

void releaseModbus5()
{
    LL_GPIO_SetOutputPin(RS485_5_DIR_Port, RS485_5_DIR_PIN);
}

/**
 * @brief 获得hModbusRTU3所在总线使用权以发送数据。
 *
 */
void getModbus3()
{
    LL_GPIO_ResetOutputPin(RS485_3_DIR_Port, RS485_3_DIR_PIN);
}

void getModbus4()
{
    LL_GPIO_ResetOutputPin(RS485_4_DIR_Port, RS485_4_DIR_PIN);
}

void getModbus5()
{
    LL_GPIO_ResetOutputPin(RS485_5_DIR_Port, RS485_5_DIR_PIN);
}

void MODH3_RxTimeOut()
{
    MODH_RxTimeOut(hModbusRTU3);
}

void MODH4_RxTimeOut()
{
    MODH_RxTimeOut(hModbusRTU4);
}

void MODH5_RxTimeOut()
{
    MODH_RxTimeOut(hModbusRTU5);
}

void gpio_initModbus3()
{
    LL_GPIO_InitTypeDef GPIO_InitStruct = {0};
    RS485_3_DIR_Port_CLK_EN();
    releaseModbus3();
    /**/
    GPIO_InitStruct.Pin        = RS485_3_DIR_PIN;
    GPIO_InitStruct.Mode       = LL_GPIO_MODE_OUTPUT;
    GPIO_InitStruct.Speed      = LL_GPIO_SPEED_FREQ_LOW;
    GPIO_InitStruct.OutputType = LL_GPIO_OUTPUT_PUSHPULL;
    GPIO_InitStruct.Pull       = LL_GPIO_PULL_NO;
    LL_GPIO_Init(RS485_3_DIR_Port, &GPIO_InitStruct);
    // 初始化时释放RS485总线
    releaseModbus3();
}

void gpio_initModbus4()
{
    LL_GPIO_InitTypeDef GPIO_InitStruct = {0};
    RS485_4_DIR_Port_CLK_EN();
    releaseModbus4();
    /**/
    GPIO_InitStruct.Pin        = RS485_4_DIR_PIN;
    GPIO_InitStruct.Mode       = LL_GPIO_MODE_OUTPUT;
    GPIO_InitStruct.Speed      = LL_GPIO_SPEED_FREQ_LOW;
    GPIO_InitStruct.OutputType = LL_GPIO_OUTPUT_PUSHPULL;
    GPIO_InitStruct.Pull       = LL_GPIO_PULL_NO;
    LL_GPIO_Init(RS485_4_DIR_Port, &GPIO_InitStruct);
    // 初始化时释放RS485总线
    releaseModbus4();
}

void gpio_initModbus5()
{
    LL_GPIO_InitTypeDef GPIO_InitStruct = {0};
    RS485_5_DIR_Port_CLK_EN();
    releaseModbus5();
    /**/
    GPIO_InitStruct.Pin        = RS485_5_DIR_PIN;
    GPIO_InitStruct.Mode       = LL_GPIO_MODE_OUTPUT;
    GPIO_InitStruct.Speed      = LL_GPIO_SPEED_FREQ_LOW;
    GPIO_InitStruct.OutputType = LL_GPIO_OUTPUT_PUSHPULL;
    GPIO_InitStruct.Pull       = LL_GPIO_PULL_NO;
    LL_GPIO_Init(RS485_5_DIR_Port, &GPIO_InitStruct);
    // 初始化时释放RS485总线
    releaseModbus5();
}

/*
*********************************************************************************************************
*	函 数 名: MODH3_ReciveNewFromISR
*	功能说明: 串口接收中断服务程序会调用本函数。当收到一个字节时，执行一次本函数。
*	形    参: 接收数据
*	返 回 值: 1 表示有数据
*********************************************************************************************************
*/
void MODH3_ReciveNewFromISR(uint8_t _data)
{
    ModbusRTUInstance_t *hmodbusRTU = hModbusRTU3;
    if (hmodbusRTU->transStageState == MODBUS_RTU_TRANS_STATE_TRANSMITING) {
        HDL_G4_CPU_Time_StartHardTimer(1, hmodbusRTU->usTimeOut35, (void *)(hmodbusRTU->t35_timeout_callback));

        if (hmodbusRTU->_RxCount < H_RX_BUF_SIZE) {
            hmodbusRTU->RxBuf[hmodbusRTU->_RxCount++] = _data;
        }
    }
}

void MODH4_ReciveNewFromISR(uint8_t _data)
{
    ModbusRTUInstance_t *hmodbusRTU = hModbusRTU4;
    if (hmodbusRTU->transStageState == MODBUS_RTU_TRANS_STATE_TRANSMITING) {
        HDL_G4_CPU_Time_StartHardTimer(2, hmodbusRTU->usTimeOut35, (void *)(hmodbusRTU->t35_timeout_callback));

        if (hmodbusRTU->_RxCount < H_RX_BUF_SIZE) {
            hmodbusRTU->RxBuf[hmodbusRTU->_RxCount++] = _data;
        }
    }
}

void MODH5_ReciveNewFromISR(uint8_t _data)
{
    ModbusRTUInstance_t *hmodbusRTU = hModbusRTU5;
    // TODO: 有些架构下即使数据还没有处理，也要请求返回数据。但是这里我认为返回的数据至少是需要存储的，即使什么也不做
    // 也可以当作一件事情，所以只有传输中可以接收下位机的数据
    if (hmodbusRTU->transStageState == MODBUS_RTU_TRANS_STATE_TRANSMITING) {
        HDL_G4_CPU_Time_StartHardTimer(3, hmodbusRTU->usTimeOut35, (void *)(hmodbusRTU->t35_timeout_callback));
        if (hmodbusRTU->_RxCount < H_RX_BUF_SIZE) {
            hmodbusRTU->RxBuf[hmodbusRTU->_RxCount++] = _data;
        }
    }
}

#define MODBUS_RTUS_HOST_NUM 3
ModbusRTUInstance_t modbusRTUList[MODBUS_RTUS_HOST_NUM] =
    {
        {
            .com                   = COM3,
            .enabled               = 0,
            .get_bus               = getModbus3,
            .release_bus           = releaseModbus3,
            .gpio_init             = gpio_initModbus3,
            .receive_char_callback = MODH3_ReciveNewFromISR,
            .t35_timeout_callback  = MODH3_RxTimeOut,

        },
        {
            .com                   = COM4,
            .enabled               = 0,
            .get_bus               = getModbus4,
            .release_bus           = releaseModbus4,
            .gpio_init             = gpio_initModbus4,
            .receive_char_callback = MODH4_ReciveNewFromISR,
            .t35_timeout_callback  = MODH4_RxTimeOut,

        },
        {
            .com                   = COM5,
            .enabled               = 0,
            .get_bus               = getModbus5,
            .release_bus           = releaseModbus5,
            .gpio_init             = gpio_initModbus5,
            .receive_char_callback = MODH5_ReciveNewFromISR,
            .t35_timeout_callback  = MODH5_RxTimeOut,
        },
};

/* COM3 DIR:PE12 HIGH:REV;LOW:SEND*/
ModbusRTUInstance_t *hModbusRTU3 = &modbusRTUList[0];

/* COM4 DIR:PE10 HIGH:REV;LOW:SEND*/
ModbusRTUInstance_t *hModbusRTU4 = &modbusRTUList[1];

/* COM5 DIR:PE8 HIGH:REV;LOW:SEND*/
ModbusRTUInstance_t *hModbusRTU5 = &modbusRTUList[2];

/*
*********************************************************************************************************
*	函 数 名: BEBufToUint16
*	功能说明: 将2字节数组(大端Big Endian次序，高字节在前)转换为16位整数
*	形    参: _pBuf : 数组
*	返 回 值: 16位整数值
*
*   大端(Big Endian)与小端(Little Endian)
*********************************************************************************************************
*/
uint16_t BEBufToUint16(uint8_t *_pBuf)
{
    return (((uint16_t)_pBuf[0] << 8) | _pBuf[1]);
}

/*
*********************************************************************************************************
*	                                   变量
*********************************************************************************************************
*/
#define NUM 1 /* 循环发送次数 */

/*
Baud rate	Bit rate	 Bit time	 Character time	  3.5 character times
  2400	    2400 bits/s	  417 us	      4.6 ms	      16 ms
  4800	    4800 bits/s	  208 us	      2.3 ms	      8.0 ms
  9600	    9600 bits/s	  104 us	      1.2 ms	      4.0 ms
 19200	   19200 bits/s    52 us	      573 us	      2.0 ms
 38400	   38400 bits/s	   26 us	      286 us	      1.75 ms(1.0 ms)
 115200	   115200 bit/s	  8.7 us	       95 us	      1.75 ms(0.33 ms) 后面固定都为1750us
*/

/*
*********************************************************************************************************
*	                                   函数声明
*********************************************************************************************************
*/
static void MODH_RxTimeOut(ModbusRTUInstance_t *hmodbusRTU);
static void MODH_AnalyzeApp(ModbusRTUInstance_t *hmodbusRTU);

static void MODH_Read_01H(ModbusRTUInstance_t *hmodbusRTU);
static void MODH_Read_02H(ModbusRTUInstance_t *hmodbusRTU);
static void MODH_Read_03H(ModbusRTUInstance_t *hmodbusRTU);
static void MODH_Read_04H(ModbusRTUInstance_t *hmodbusRTU);
static void MODH_Read_05H(ModbusRTUInstance_t *hmodbusRTU);
static void MODH_Read_06H(ModbusRTUInstance_t *hmodbusRTU);
static void MODH_Read_10H(ModbusRTUInstance_t *hmodbusRTU);

/*
*********************************************************************************************************
*	函 数 名: MODH_SendPacket
*	功能说明: 发送数据包 COM1口
*	形    参: _buf : 数据缓冲区
*			  _len : 数据长度
*	返 回 值: 无
*********************************************************************************************************
*/
void MODH_SendPacket(ModbusRTUInstance_t *hmodbusRTU, uint8_t *_buf, uint16_t _len)
{
    RS485_SendBuf(hmodbusRTU, _buf, _len);
}

/*
*********************************************************************************************************
*	函 数 名: MODH_SendAckWithCRC
*	功能说明: 发送应答,自动加CRC.
*	形    参: 无。
*	返 回 值: 无
*********************************************************************************************************
*/
static void MODH_SendAckWithCRC(ModbusRTUInstance_t *hmodbusRTU)
{
    uint16_t crc;

    crc                                       = CRC16_Modbus(hmodbusRTU->TxBuf, hmodbusRTU->_TxCount);
    hmodbusRTU->TxBuf[hmodbusRTU->_TxCount++] = crc >> 8;
    hmodbusRTU->TxBuf[hmodbusRTU->_TxCount++] = crc;
    MODH_SendPacket(hmodbusRTU, hmodbusRTU->TxBuf, hmodbusRTU->_TxCount);
}

/*
*********************************************************************************************************
*	函 数 名: MODH_AnalyzeApp
*	功能说明: 分析应用层协议。处理应答。
*	形    参: 无
*	返 回 值: 无
*********************************************************************************************************
*/
static void MODH_AnalyzeApp(ModbusRTUInstance_t *hmodbusRTU)
{
    hmodbusRTU->respons_func_code = hmodbusRTU->RxBuf[1];
    switch (hmodbusRTU->RxBuf[1]) /* 第2个字节 功能码 */
    {
        case 0x01: /* 读取线圈状态 */
            MODH_Read_01H(hmodbusRTU);
            break;

        case 0x02: /* 读取输入状态 */
            MODH_Read_02H(hmodbusRTU);
            break;

        case 0x03: /* 读取保持寄存器 在一个或多个保持寄存器中取得当前的二进制值 */
            MODH_Read_03H(hmodbusRTU);
            break;

        case 0x04: /* 读取输入寄存器 */
            MODH_Read_04H(hmodbusRTU);
            break;

        case 0x05: /* 强制单线圈 */
            MODH_Read_05H(hmodbusRTU);
            break;

        case 0x06: /* 写单个寄存器 */
            MODH_Read_06H(hmodbusRTU);
            break;

        case 0x10: /* 写多个寄存器 */
            MODH_Read_10H(hmodbusRTU);
            break;

        default:
            break;
    }
}

/*
*********************************************************************************************************
*	函 数 名: MODH_Send01H
*	功能说明: 发送01H指令，查询1个或多个线圈寄存器
*	形    参: _addr : 从站地址
*			  _reg : 寄存器编号
*			  _num : 寄存器个数
*	返 回 值: 无
*********************************************************************************************************
*/
void MODH_Send01H(ModbusRTUInstance_t *hmodbusRTU, uint8_t _addr, uint16_t _reg, uint16_t _num)
{
    hmodbusRTU->func_code = 0x01;

    hmodbusRTU->_TxCount                      = 0;
    hmodbusRTU->TxBuf[hmodbusRTU->_TxCount++] = _addr;     /* 从站地址 */
    hmodbusRTU->TxBuf[hmodbusRTU->_TxCount++] = 0x01;      /* 功能码 */
    hmodbusRTU->TxBuf[hmodbusRTU->_TxCount++] = _reg >> 8; /* 寄存器编号 高字节 */
    hmodbusRTU->TxBuf[hmodbusRTU->_TxCount++] = _reg;      /* 寄存器编号 低字节 */
    hmodbusRTU->TxBuf[hmodbusRTU->_TxCount++] = _num >> 8; /* 寄存器个数 高字节 */
    hmodbusRTU->TxBuf[hmodbusRTU->_TxCount++] = _num;      /* 寄存器个数 低字节 */

    MODH_SendAckWithCRC(hmodbusRTU); /* 发送数据，自动加CRC */

    hmodbusRTU->transEvent = MODBUS_RTU_TRANS_RES_NONE; /* 清接传输结果标志 */
    hmodbusRTU->RegNum     = _num;                      /* 寄存器个数 */
    hmodbusRTU->RegAddr    = _reg;                      /* 保存01H指令中的寄存器地址，方便对应答数据进行分类 */
    hmodbusRTU->_RxCount   = 0;                         /*清空接收缓存*/
    hmodbusRTU->slaveAddr  = _addr;                     /*记录本次传输的从机地址*/
}

/*
*********************************************************************************************************
*	函 数 名: MODH_Send02H
*	功能说明: 发送02H指令，读离散输入寄存器
*	形    参: _addr : 从站地址
*			  _reg : 寄存器编号
*			  _num : 寄存器个数
*	返 回 值: 无
*********************************************************************************************************
*/
void MODH_Send02H(ModbusRTUInstance_t *hmodbusRTU, uint8_t _addr, uint16_t _reg, uint16_t _num)
{
    hmodbusRTU->func_code                     = 0x02;
    hmodbusRTU->_TxCount                      = 0;
    hmodbusRTU->TxBuf[hmodbusRTU->_TxCount++] = _addr;     /* 从站地址 */
    hmodbusRTU->TxBuf[hmodbusRTU->_TxCount++] = 0x02;      /* 功能码 */
    hmodbusRTU->TxBuf[hmodbusRTU->_TxCount++] = _reg >> 8; /* 寄存器编号 高字节 */
    hmodbusRTU->TxBuf[hmodbusRTU->_TxCount++] = _reg;      /* 寄存器编号 低字节 */
    hmodbusRTU->TxBuf[hmodbusRTU->_TxCount++] = _num >> 8; /* 寄存器个数 高字节 */
    hmodbusRTU->TxBuf[hmodbusRTU->_TxCount++] = _num;      /* 寄存器个数 低字节 */

    MODH_SendAckWithCRC(hmodbusRTU);                    /* 发送数据，自动加CRC */
    hmodbusRTU->transEvent = MODBUS_RTU_TRANS_RES_NONE; /* 清接传输结果标志 */
    hmodbusRTU->RegNum     = _num;                      /* 寄存器个数 */
    hmodbusRTU->RegAddr    = _reg;                      /* 保存02H指令中的寄存器地址，方便对应答数据进行分类 */
    hmodbusRTU->_RxCount   = 0;                         /*清空接收缓存*/
    hmodbusRTU->slaveAddr  = _addr;                     /*记录本次传输的从机地址*/
}

/*
*********************************************************************************************************
*	函 数 名: MODH_Send03H
*	功能说明: 发送03H指令，查询1个或多个保持寄存器
*	形    参: _addr : 从站地址
*			  _reg : 寄存器编号
*			  _num : 寄存器个数
*	返 回 值: 无
*********************************************************************************************************
*/
void MODH_Send03H(ModbusRTUInstance_t *hmodbusRTU, uint8_t _addr, uint16_t _reg, uint16_t _num)
{
    hmodbusRTU->func_code                     = 0x03;
    hmodbusRTU->_TxCount                      = 0;
    hmodbusRTU->TxBuf[hmodbusRTU->_TxCount++] = _addr;     /* 从站地址 */
    hmodbusRTU->TxBuf[hmodbusRTU->_TxCount++] = 0x03;      /* 功能码 */
    hmodbusRTU->TxBuf[hmodbusRTU->_TxCount++] = _reg >> 8; /* 寄存器编号 高字节 */
    hmodbusRTU->TxBuf[hmodbusRTU->_TxCount++] = _reg;      /* 寄存器编号 低字节 */
    hmodbusRTU->TxBuf[hmodbusRTU->_TxCount++] = _num >> 8; /* 寄存器个数 高字节 */
    hmodbusRTU->TxBuf[hmodbusRTU->_TxCount++] = _num;      /* 寄存器个数 低字节 */

    MODH_SendAckWithCRC(hmodbusRTU);                    /* 发送数据，自动加CRC */
    hmodbusRTU->transEvent = MODBUS_RTU_TRANS_RES_NONE; /* 清接传输结果标志 */
    hmodbusRTU->RegNum     = _num;                      /* 寄存器个数 */
    hmodbusRTU->RegAddr    = _reg;                      /* 保存03H指令中的寄存器地址，方便对应答数据进行分类 */
    hmodbusRTU->_RxCount   = 0;                         /*清空接收缓存*/
    hmodbusRTU->slaveAddr  = _addr;                     /*记录本次传输的从机地址*/
}

/*
*********************************************************************************************************
*	函 数 名: MODH_Send04H
*	功能说明: 发送04H指令，读输入寄存器
*	形    参: _addr : 从站地址
*			  _reg : 寄存器编号
*			  _num : 寄存器个数
*	返 回 值: 无
*********************************************************************************************************
*/
void MODH_Send04H(ModbusRTUInstance_t *hmodbusRTU, uint8_t _addr, uint16_t _reg, uint16_t _num)
{
    hmodbusRTU->func_code                     = 0x04;
    hmodbusRTU->_TxCount                      = 0;
    hmodbusRTU->TxBuf[hmodbusRTU->_TxCount++] = _addr;     /* 从站地址 */
    hmodbusRTU->TxBuf[hmodbusRTU->_TxCount++] = 0x04;      /* 功能码 */
    hmodbusRTU->TxBuf[hmodbusRTU->_TxCount++] = _reg >> 8; /* 寄存器编号 高字节 */
    hmodbusRTU->TxBuf[hmodbusRTU->_TxCount++] = _reg;      /* 寄存器编号 低字节 */
    hmodbusRTU->TxBuf[hmodbusRTU->_TxCount++] = _num >> 8; /* 寄存器个数 高字节 */
    hmodbusRTU->TxBuf[hmodbusRTU->_TxCount++] = _num;      /* 寄存器个数 低字节 */

    MODH_SendAckWithCRC(hmodbusRTU);                    /* 发送数据，自动加CRC */
    hmodbusRTU->transEvent = MODBUS_RTU_TRANS_RES_NONE; /* 清接传输结果标志 */
    hmodbusRTU->RegNum     = _num;                      /* 寄存器个数 */
    hmodbusRTU->RegAddr    = _reg;                      /* 保存04H指令中的寄存器地址，方便对应答数据进行分类 */
    hmodbusRTU->_RxCount   = 0;                         /*清空接收缓存*/
    hmodbusRTU->slaveAddr  = _addr;                     /*记录本次传输的从机地址*/
}

/*
*********************************************************************************************************
*	函 数 名: MODH_Send05H
*	功能说明: 发送05H指令，写强置单线圈
*	形    参: _addr : 从站地址
*			  _reg : 寄存器编号
*			  _value : 寄存器值,2字节
*	返 回 值: 无
*********************************************************************************************************
*/
void MODH_Send05H(ModbusRTUInstance_t *hmodbusRTU, uint8_t _addr, uint16_t _reg, uint16_t _value)
{
    hmodbusRTU->func_code                     = 0x05;
    hmodbusRTU->_TxCount                      = 0;
    hmodbusRTU->TxBuf[hmodbusRTU->_TxCount++] = _addr;       /* 从站地址 */
    hmodbusRTU->TxBuf[hmodbusRTU->_TxCount++] = 0x05;        /* 功能码 */
    hmodbusRTU->TxBuf[hmodbusRTU->_TxCount++] = _reg >> 8;   /* 寄存器编号 高字节 */
    hmodbusRTU->TxBuf[hmodbusRTU->_TxCount++] = _reg;        /* 寄存器编号 低字节 */
    hmodbusRTU->TxBuf[hmodbusRTU->_TxCount++] = _value >> 8; /* 寄存器值 高字节 */
    hmodbusRTU->TxBuf[hmodbusRTU->_TxCount++] = _value;      /* 寄存器值 低字节 */

    MODH_SendAckWithCRC(hmodbusRTU); /* 发送数据，自动加CRC */

    hmodbusRTU->transEvent = MODBUS_RTU_TRANS_RES_NONE; /* 清接传输结果标志 */
    hmodbusRTU->_RxCount   = 0;                         /*清空接收缓存*/
    hmodbusRTU->slaveAddr  = _addr;                     /*记录本次传输的从机地址*/
}

/*
*********************************************************************************************************
*	函 数 名: MODH_Send06H
*	功能说明: 发送06H指令，写1个保持寄存器
*	形    参: _addr : 从站地址
*			  _reg : 寄存器编号
*			  _value : 寄存器值,2字节
*	返 回 值: 无
*********************************************************************************************************
*/
void MODH_Send06H(ModbusRTUInstance_t *hmodbusRTU, uint8_t _addr, uint16_t _reg, uint16_t _value)
{
    hmodbusRTU->func_code                     = 0x06;
    hmodbusRTU->_TxCount                      = 0;
    hmodbusRTU->TxBuf[hmodbusRTU->_TxCount++] = _addr;       /* 从站地址 */
    hmodbusRTU->TxBuf[hmodbusRTU->_TxCount++] = 0x06;        /* 功能码 */
    hmodbusRTU->TxBuf[hmodbusRTU->_TxCount++] = _reg >> 8;   /* 寄存器编号 高字节 */
    hmodbusRTU->TxBuf[hmodbusRTU->_TxCount++] = _reg;        /* 寄存器编号 低字节 */
    hmodbusRTU->TxBuf[hmodbusRTU->_TxCount++] = _value >> 8; /* 寄存器值 高字节 */
    hmodbusRTU->TxBuf[hmodbusRTU->_TxCount++] = _value;      /* 寄存器值 低字节 */

    MODH_SendAckWithCRC(hmodbusRTU); /* 发送数据，自动加CRC */

    hmodbusRTU->transEvent = MODBUS_RTU_TRANS_RES_NONE; /* 清接传输结果标志 */
    hmodbusRTU->_RxCount   = 0;                         /*清空接收缓存*/
    hmodbusRTU->slaveAddr  = _addr;                     /*记录本次传输的从机地址*/
}

/*
*********************************************************************************************************
*	函 数 名: MODH_Send10H
*	功能说明: 发送10H指令，连续写多个保持寄存器. 最多一次支持23个寄存器。
*	形    参: _addr : 从站地址
*			  _reg : 寄存器编号
*			  _num : 寄存器个数n (每个寄存器2个字节) 值域
*			  _buf : n个寄存器的数据。长度 = 2 * n
*	返 回 值: 无
*********************************************************************************************************
*/
void MODH_Send10H(ModbusRTUInstance_t *hmodbusRTU, uint8_t _addr, uint16_t _reg, uint8_t _num, uint8_t *_buf)
{
    uint16_t i;
    hmodbusRTU->func_code                     = 0x10;
    hmodbusRTU->_TxCount                      = 0;
    hmodbusRTU->TxBuf[hmodbusRTU->_TxCount++] = _addr;     /* 从站地址 */
    hmodbusRTU->TxBuf[hmodbusRTU->_TxCount++] = 0x10;      /* 从站地址 */
    hmodbusRTU->TxBuf[hmodbusRTU->_TxCount++] = _reg >> 8; /* 寄存器编号 高字节 */
    hmodbusRTU->TxBuf[hmodbusRTU->_TxCount++] = _reg;      /* 寄存器编号 低字节 */
    hmodbusRTU->TxBuf[hmodbusRTU->_TxCount++] = _num >> 8; /* 寄存器个数 高字节 */
    hmodbusRTU->TxBuf[hmodbusRTU->_TxCount++] = _num;      /* 寄存器个数 低字节 */
    hmodbusRTU->TxBuf[hmodbusRTU->_TxCount++] = 2 * _num;  /* 数据字节数 */

    for (i = 0; i < 2 * _num; i++) {
        if (hmodbusRTU->_TxCount > H_RX_BUF_SIZE - 3) {
            return; /* 数据超过缓冲区超度，直接丢弃不发送 */
        }
        hmodbusRTU->TxBuf[hmodbusRTU->_TxCount++] = _buf[i]; /* 后面的数据长度 */
    }

    MODH_SendAckWithCRC(hmodbusRTU);                    /* 发送数据，自动加CRC */
    hmodbusRTU->transEvent = MODBUS_RTU_TRANS_RES_NONE; /* 清接传输结果标志 */
    hmodbusRTU->_RxCount   = 0;                         /*清空接收缓存*/
    hmodbusRTU->slaveAddr  = _addr;                     /*记录本次传输的从机地址*/
}

/*
*********************************************************************************************************
*	函 数 名: MODH_ReciveNew
*	功能说明: 串口接收中断服务程序会调用本函数。当收到一个字节时，执行一次本函数。
*	形    参: 接收数据
*	返 回 值: 1 表示有数据
*********************************************************************************************************
*/
void MODH_ReciveNew(ModbusRTUInstance_t *hmodbusRTU, uint8_t _data)
{
    /*
        3.5个字符的时间间隔，只是用在RTU模式下面，因为RTU模式没有开始符和结束符，
        两个数据包之间只能靠时间间隔来区分，Modbus定义在不同的波特率下，间隔时间是不一样的，
        详情看此C文件开头
    */

    /*记录上一次收到一个字节的微秒时间戳*/
    if (hmodbusRTU->transStageState == MODBUS_RTU_TRANS_STATE_TRANSMITING) {
        hmodbusRTU->lastRevByteUsTick = HDL_G4_CPU_Time_GetUsTick();

        if (hmodbusRTU->_RxCount < H_RX_BUF_SIZE) {
            hmodbusRTU->RxBuf[hmodbusRTU->_RxCount++] = _data;
        }
    }
}

/*
*********************************************************************************************************
*	函 数 名: MODH_RxTimeOut
*	功能说明: 超过3.5个字符时间后执行本函数。 设置全局变量 g_rtu_timeout = 1; 通知主程序开始解码。
*	形    参: 无
*	返 回 值: 无
*********************************************************************************************************
*/
static inline void MODH_RxTimeOut(ModbusRTUInstance_t *hmodbusRTU)
{
    hmodbusRTU->gTimeOutT35Flag = 1;
}

/*
*********************************************************************************************************
*	函 数 名: modbus_rtu_poll
*	功能说明: 接收控制器指令. 1ms 响应时间。
*	形    参: 无
*	返 回 值: 0 表示无数据 1表示收到正确命令
*********************************************************************************************************
*/
void modbus_rtu_poll(void)
{
    for (size_t i = 0; i < MODBUS_RTUS_HOST_NUM; i++) {
        ModbusRTUInstance_t *hmodbusRTU = &modbusRTUList[i];
        // Modbus RTU host未使能,掠过执行
        if (!hmodbusRTU->enabled) {
            continue;
        }

        uint16_t crc1;

        if (hmodbusRTU->gTimeOutT35Flag == 0) /* 超过3.5个字符时间后执行MODH_RxTimeOut()函数。全局变量 g_rtu_timeout = 1 */
        {
            /* 没有超时，继续接收。不要清零 hmodbusRTU->RxCount */
            continue;
        }
        // else
        {
            /* 收到命令
                05 06 00 88 04 57 3B70 (8 字节)
                    05    :  数码管屏的号站，
                    06    :  指令
                    00 88 :  数码管屏的显示寄存器
                    04 57 :  数据,,,转换成 10 进制是 1111.高位在前,
                    3B70  :  二个字节 CRC 码	从05到 57的校验
            */
            hmodbusRTU->gTimeOutT35Flag = 0;

            /* 接收到的数据小于4个字节就认为错误，地址（8bit）+指令（8bit）+操作寄存器（16bit） */
            if (hmodbusRTU->_RxCount < 4) {
                hmodbusRTU->_RxCount = 0; /* 必须清零计数器，方便下次帧同步 */

                hmodbusRTU->transEvent = MODBUS_RTU_TRANS_ERR_REV_FRME; /* 帧错误 */
                MODBUS_RTU_SET_TRANS_STATE(hmodbusRTU, MODBUS_RTU_TRANS_STATE_WAITING_DISPOSE);
                continue;
            }

            /* 计算CRC校验和，这里是将接收到的数据包含CRC16值一起做CRC16，结果是0，表示正确接收 */
            crc1 = CRC16_Modbus(hmodbusRTU->RxBuf, hmodbusRTU->_RxCount);
            if (crc1 != 0) {

                char buf[24 * 3];
                int show_len = hmodbusRTU->_RxCount > sizeof(buf) ? sizeof(buf) : hmodbusRTU->_RxCount;
                for (size_t i = 0; i < show_len; i++) {
                    sprintf(buf + i * 3, "%02X ", hmodbusRTU->RxBuf[i]);
                }
                ULOG_ERROR("[Modbus][CRC] check error! crc = 0x%04X, _RxCount = %d,COM : %d ADDR %d reg addr:%d Rev: %s ", crc1, hmodbusRTU->_RxCount,
                           hmodbusRTU->com, hmodbusRTU->slaveAddr, hmodbusRTU->RegAddr, buf);

                hmodbusRTU->_RxCount = 0; /* 必须清零计数器，方便下次帧同步 */

                hmodbusRTU->transEvent = MODBUS_RTU_TRANS_ERR_REV_FRME; /* 帧错误 */
                MODBUS_RTU_SET_TRANS_STATE(hmodbusRTU, MODBUS_RTU_TRANS_STATE_WAITING_DISPOSE);
                continue;
            }

            /* 分析应用层协议：
                    内部会清空接收缓存的数据(hmodbusRTU->_RxCount = 0;),
                    这样的好处是modbus_rtu_poll这就只会在第一次收到完整回复时计算CRC以及
                    进行数据解析等工作。
            */
            MODH_AnalyzeApp(hmodbusRTU);
        }
    }
}

/**
 * @brief 产生Modbus命令超时事件。
 * 因为Modbus是发送一个命令就必须等到相应的回复或者超时才能继续下一次命令，
 * 所以这里定义一次传输就是命令到收到回复，注意不是到收到回复并处理完回复。
 */
void modbus_rtu_timeout_handler(void)
{
    for (size_t i = 0; i < MODBUS_RTUS_HOST_NUM; i++) {
        ModbusRTUInstance_t *hmodbusRTU = &modbusRTUList[i];
        // Modbus RTU host未使能,掠过执行
        if (!hmodbusRTU->enabled) {
            continue;
        }

        if (hmodbusRTU->transStageState == MODBUS_RTU_TRANS_STATE_TRANSMITING) // 在传输中才计时
        {
            // 传输超时处理，因为Modbus是发送一个命令就必须等到相应的回复或者超时才能继续下一次命令，
            // 所以这里定义一次传输就是命令到收到回复，注意不是到收到回复并处理完回复。
            if ((HDL_G4_CPU_Time_GetTick() - hmodbusRTU->tickstart) > hmodbusRTU->TIMEOUT) {
                hmodbusRTU->transEvent = MODBUS_RTU_TRANS_ERR_TRANS_TIMEOUT; /* 通信超时了 */
                // 超时，传输结束，转变为等待处理传输
                MODBUS_RTU_SET_TRANS_STATE(hmodbusRTU, MODBUS_RTU_TRANS_STATE_WAITING_DISPOSE);
            }
        }
    }
}

void modbus_rtu_trans_event_inner_handler()
{
    for (size_t i = 0; i < MODBUS_RTUS_HOST_NUM; i++) {
        ModbusRTUInstance_t *hmodbusRTU = &modbusRTUList[i];
        // Modbus RTU host未使能,掠过执行
        if (!hmodbusRTU->enabled) {
            continue;
        }

        if (modbus_rtu_host_have_trans_event(hmodbusRTU)) {
            modbus_rtu_trans_event_inner_deal(hmodbusRTU);
        }
    }
}

// 这里进行超时处理
void modbus_rtu_trans_event_inner_deal(ModbusRTUInstance_t *hmodbusRTU)
{
    switch (hmodbusRTU->transEvent) {
        case MODBUS_RTU_TRANS_REV_ACK:
            // TODO:这里按理来说还应该匹配一些从设备的故障消息(MODBUS 异常码)
            if (hmodbusRTU->respons_func_code > 0x80) {
                MODBUS_RTU_RESET_TRANS_STATE(hmodbusRTU);
                hmodbusRTU->transEvent = MODBUS_RTU_TRANS_RES_NONE;
#if MOD_BUS_DEBUG == 1
                ULOG_ERROR("[Modbus RTU]: MODBUS RTU TRANS RECEIVE EXCEPTION CODE COM : %d ADDR %d reg addr:%d", hmodbusRTU->com, hmodbusRTU->slaveAddr, hmodbusRTU->RegAddr);
#endif // !MOD_BUS_DEBUG
            }
            break;
        case MODBUS_RTU_TRANS_ERR_TRANS_TIMEOUT:
            MODBUS_RTU_RESET_TRANS_STATE(hmodbusRTU);
            hmodbusRTU->transEvent = MODBUS_RTU_TRANS_RES_NONE;
            if (hmodbusRTU->innerTimeoutEventHandler != NULL) {
                hmodbusRTU->innerTimeoutEventHandler(hmodbusRTU);
            }
#if MOD_BUS_DEBUG == 1
            ULOG_ERROR("[Modbus RTU]: MODBUS_RTU_TRANS_ERR_TRANS_TIMEOUT COM : %d ADDR %d reg addr:%d", hmodbusRTU->com, hmodbusRTU->slaveAddr, hmodbusRTU->RegAddr);
#endif // !MOD_BUS_DEBUG
            break;
        case MODBUS_RTU_TRANS_ERR_REV_FRME:
            MODBUS_RTU_RESET_TRANS_STATE(hmodbusRTU);
            hmodbusRTU->transEvent = MODBUS_RTU_TRANS_RES_NONE;
#if MOD_BUS_DEBUG == 1
            ULOG_ERROR("[Modbus RTU]: MODBUS_RTU_TRANS_ERR_REV_FRME COM : %d ADDR %d reg addr:%d", hmodbusRTU->com, hmodbusRTU->slaveAddr, hmodbusRTU->RegAddr);
#endif // !MOD_BUS_DEBUG
            break;
        default:
            break;
    }
}

/*
*********************************************************************************************************
*	函 数 名: MODH_Read_01H
*	功能说明: 分析01H指令的应答数据，读取线圈状态，bit访问
*	形    参: 无
*	返 回 值: 无
*********************************************************************************************************
*/
static void MODH_Read_01H(ModbusRTUInstance_t *hmodbusRTU)
{
    if (hmodbusRTU->_RxCount > 0) {

        hmodbusRTU->transEvent = MODBUS_RTU_TRANS_REV_ACK;
        // hmodbusRTU->fAck01H_bytes = hmodbusRTU->RxBuf[2]; /* 数据长度 字节数 */
        // hmodbusRTU->fAck01H_data_ptr = &hmodbusRTU->RxBuf[3];
        hmodbusRTU->RxCount = hmodbusRTU->_RxCount;
        // 清除内部接收缓存的数据,这样的好处是modbus_rtu_poll这就只会在第一次收到完整回复时计算CRC以及
        // 进行数据解析等工作。
        hmodbusRTU->_RxCount = 0;
        MODBUS_RTU_SET_TRANS_STATE(hmodbusRTU, MODBUS_RTU_TRANS_STATE_WAITING_DISPOSE);
    }
}

/*
*********************************************************************************************************
*	函 数 名: MODH_Read_02H
*	功能说明: 分析02H指令的应答数据，读取输入状态，bit访问
*	形    参: 无
*	返 回 值: 无
*********************************************************************************************************
*/
static void MODH_Read_02H(ModbusRTUInstance_t *hmodbusRTU)
{
    if (hmodbusRTU->_RxCount > 0) {
        hmodbusRTU->transEvent = MODBUS_RTU_TRANS_REV_ACK;
        // hmodbusRTU->fAck02H_bytes = hmodbusRTU->RxBuf[2]; /* 数据长度 字节数 */
        // hmodbusRTU->fAck02H_data_ptr = &hmodbusRTU->RxBuf[3];
        hmodbusRTU->RxCount  = hmodbusRTU->_RxCount;
        hmodbusRTU->_RxCount = 0;
        MODBUS_RTU_SET_TRANS_STATE(hmodbusRTU, MODBUS_RTU_TRANS_STATE_WAITING_DISPOSE);
    }
}

/*
*********************************************************************************************************
*	函 数 名: MODH_Read_04H
*	功能说明: 分析04H指令的应答数据，读取输入寄存器，16bit访问
*	形    参: 无
*	返 回 值: 无
*********************************************************************************************************
*/
static void MODH_Read_04H(ModbusRTUInstance_t *hmodbusRTU)
{
    if (hmodbusRTU->_RxCount > 0) {
        hmodbusRTU->transEvent = MODBUS_RTU_TRANS_REV_ACK;
        // hmodbusRTU->fAck04H_bytes = hmodbusRTU->RxBuf[2]; /* 数据长度 字节数 */
        // hmodbusRTU->fAck04H_data_ptr = &hmodbusRTU->RxBuf[3];
        hmodbusRTU->RxCount  = hmodbusRTU->_RxCount;
        hmodbusRTU->_RxCount = 0;
        MODBUS_RTU_SET_TRANS_STATE(hmodbusRTU, MODBUS_RTU_TRANS_STATE_WAITING_DISPOSE);
    }
}

/*
*********************************************************************************************************
*	函 数 名: MODH_Read_05H
*	功能说明: 分析05H指令的应答数据，写入线圈状态，bit访问
*	形    参: 无
*	返 回 值: 无
*********************************************************************************************************
*/
static void MODH_Read_05H(ModbusRTUInstance_t *hmodbusRTU)
{
    if (hmodbusRTU->_RxCount > 0) {
        if (hmodbusRTU->RxBuf[0] == hmodbusRTU->slaveAddr) {
            hmodbusRTU->transEvent = MODBUS_RTU_TRANS_REV_ACK; /* 接收到应答 */
            hmodbusRTU->RxCount    = hmodbusRTU->_RxCount;
            hmodbusRTU->_RxCount   = 0;
            MODBUS_RTU_SET_TRANS_STATE(hmodbusRTU, MODBUS_RTU_TRANS_STATE_WAITING_DISPOSE);
        }
    };
}

/*
*********************************************************************************************************
*	函 数 名: MODH_Read_06H
*	功能说明: 分析06H指令的应答数据，写单个保存寄存器，16bit访问
*	形    参: 无
*	返 回 值: 无
*********************************************************************************************************
*/
static void MODH_Read_06H(ModbusRTUInstance_t *hmodbusRTU)
{
    if (hmodbusRTU->_RxCount > 0) {
        if (hmodbusRTU->RxBuf[0] == hmodbusRTU->slaveAddr) {
            hmodbusRTU->transEvent = MODBUS_RTU_TRANS_REV_ACK; /* 接收到应答 */
            hmodbusRTU->RxCount    = hmodbusRTU->_RxCount;
            hmodbusRTU->_RxCount   = 0;
            MODBUS_RTU_SET_TRANS_STATE(hmodbusRTU, MODBUS_RTU_TRANS_STATE_WAITING_DISPOSE);
        }
    }
}

/*
*********************************************************************************************************
*	函 数 名: MODH_Read_03H
*	功能说明: 分析03H指令的应答数据，读取保持寄存器，16bit访问
*	形    参: 无
*	返 回 值: 无
*********************************************************************************************************
*/
static void MODH_Read_03H(ModbusRTUInstance_t *hmodbusRTU)
{

    if (hmodbusRTU->_RxCount > 0) {
        hmodbusRTU->transEvent = MODBUS_RTU_TRANS_REV_ACK;
        uint8_t bytes;
        uint8_t *p;
        bytes = hmodbusRTU->RxBuf[2]; /* 数据长度 字节数 */
        p     = &hmodbusRTU->RxBuf[3];

        // 用寄存器数量来计数缓存数据，同时计算寄存器数量采样向下取整的方法，
        // 防止奇怪的数据帧进入程序
        hmodbusRTU->result.len          = bytes;
        hmodbusRTU->result.register_num = bytes / 2;
        if (hmodbusRTU->result.register_num > 4) // TODO:magic number:这个时寄存器缓存的大小
        {
            hmodbusRTU->result.register_num = 4;
        }
        for (size_t i = 0; i < hmodbusRTU->result.register_num; i++) {
            hmodbusRTU->result.P[i] = BEBufToUint16(p);
            p += 2; /* 寄存器 */
        }
        hmodbusRTU->RxCount  = hmodbusRTU->_RxCount;
        hmodbusRTU->_RxCount = 0; // 清除内部接收缓存的数据
        MODBUS_RTU_SET_TRANS_STATE(hmodbusRTU, MODBUS_RTU_TRANS_STATE_WAITING_DISPOSE);
    }
}

/*
*********************************************************************************************************
*	函 数 名: MODH_Read_10H
*	功能说明: 分析10H指令的应答数据，写多个保存寄存器，16bit访问
*	形    参: 无
*	返 回 值: 无
*********************************************************************************************************
*/
void MODH_Read_10H(ModbusRTUInstance_t *hmodbusRTU)
{
    /*
        10H指令的应答:
            从机地址                11
            功能码                  10
            寄存器起始地址高字节	00
            寄存器起始地址低字节    01
            寄存器数量高字节        00
            寄存器数量低字节        02
            CRC校验高字节           12
            CRC校验低字节           98
    */
    if (hmodbusRTU->_RxCount > 0) {
        if (hmodbusRTU->RxBuf[0] == hmodbusRTU->slaveAddr) {
            hmodbusRTU->transEvent = MODBUS_RTU_TRANS_REV_ACK; /* 接收到应答 */
            hmodbusRTU->RxCount    = hmodbusRTU->_RxCount;
            hmodbusRTU->_RxCount   = 0; // 清除内部接收缓存的数据
            MODBUS_RTU_SET_TRANS_STATE(hmodbusRTU, MODBUS_RTU_TRANS_STATE_WAITING_DISPOSE);
        }
    }
}

/*
*********************************************************************************************************
*	函 数 名: modbus_rtu_host_read_01H
*	功能说明: 单个参数. 通过发送01H指令实现，发送之后，等待从机应答。
*	形    参: 无
*	返 回 值: 1 表示成功。0 表示失败（通信超时或被拒绝）
*********************************************************************************************************
*/
uint8_t modbus_rtu_host_read_01H(ModbusRTUInstance_t *hmodbusRTU, uint8_t slaveAddr, uint16_t _reg, uint16_t _num)
{

    uint8_t i;
    uint8_t status = 0;
    // 处于空闲状态
    if (hmodbusRTU->transStageState == MODBUS_RTU_TRANS_STATE_IDLE) {
        hmodbusRTU->transStageState = MODBUS_RTU_TRANS_STATE_TRANSMITING;
    } else {
        return status;
    }

    for (i = 0; i < NUM; i++) {
        MODH_Send01H(hmodbusRTU, slaveAddr, _reg, _num);   /* 发送命令 */
        hmodbusRTU->tickstart = HDL_G4_CPU_Time_GetTick(); /* 记录命令发送的时刻 */

        while (1) /* 等待应答,超时或接收到应答则break  */
        {
            modbus_rtu_handler();

            if (hmodbusRTU->transStageState == MODBUS_RTU_TRANS_STATE_WAITING_DISPOSE) {
                break;       /* 传输结束了，需要去处理传输结果 */
                i = NUM - 1; // 跳出For循环。
            }
        }
    }
    status = hmodbusRTU->transEvent == MODBUS_RTU_TRANS_REV_ACK; /* 01H 读成功 */
    modbus_rtu_trans_event_inner_deal(hmodbusRTU);
    MODBUS_RTU_RESET_TRANS_STATE(hmodbusRTU);
    return status;
}

/*
*********************************************************************************************************
*	函 数 名: modbus_rtu_host_read_param_02H
*	功能说明: 单个参数. 通过发送02H指令实现，发送之后，等待从机应答。
*	形    参: 无
*	返 回 值: 1 表示成功。0 表示失败（通信超时或被拒绝）
*********************************************************************************************************
*/
uint8_t modbus_rtu_host_read_02H(ModbusRTUInstance_t *hmodbusRTU, uint8_t slaveAddr, uint16_t _reg, uint16_t _num)
{
    uint8_t i;
    uint8_t status = 0; // 默认失败
    // 处于空闲状态
    if (hmodbusRTU->transStageState == MODBUS_RTU_TRANS_STATE_IDLE) {
        hmodbusRTU->transStageState = MODBUS_RTU_TRANS_STATE_TRANSMITING;
    } else {
        return status;
    }

    for (i = 0; i < NUM; i++) {
        MODH_Send02H(hmodbusRTU, slaveAddr, _reg, _num);
        hmodbusRTU->tickstart = HDL_G4_CPU_Time_GetTick(); /* 记录命令发送的时刻 */

        while (1) {
            modbus_rtu_handler();
            if (hmodbusRTU->transStageState == MODBUS_RTU_TRANS_STATE_WAITING_DISPOSE) {
                break;       /* 传输结束了，需要去处理传输结果 */
                i = NUM - 1; // 跳出For循环。
            }
        }
    }
    status = hmodbusRTU->transEvent == MODBUS_RTU_TRANS_REV_ACK; /* 02H 读成功 */
    modbus_rtu_trans_event_inner_deal(hmodbusRTU);
    MODBUS_RTU_RESET_TRANS_STATE(hmodbusRTU);
    return status;
}
/*
*********************************************************************************************************
*	函 数 名: modbus_rtu_host_read_03H
*	功能说明: 单个参数. 通过发送03H指令实现，发slaveAddr送之后，等待从机应答。
*	形    参: 无
*	返 回 值: 1 表示成功。0 表示失败（通信超时或被拒绝）
*********************************************************************************************************
*/
uint8_t modbus_rtu_host_read_03H(ModbusRTUInstance_t *hmodbusRTU, uint8_t slaveAddr, uint16_t _reg, uint16_t _num)
{

    uint8_t i;
    uint8_t status = 0; // 默认失败
    // 处于空闲状态
    if (hmodbusRTU->transStageState == MODBUS_RTU_TRANS_STATE_IDLE) {
        hmodbusRTU->transStageState = MODBUS_RTU_TRANS_STATE_TRANSMITING;
    } else {
        return status;
    }

    for (i = 0; i < NUM; i++) {
        MODH_Send03H(hmodbusRTU, slaveAddr, _reg, _num);
        hmodbusRTU->tickstart = HDL_G4_CPU_Time_GetTick(); /* 记录命令发送的时刻 */

        while (1) {
            modbus_rtu_handler();
            if (hmodbusRTU->transStageState == MODBUS_RTU_TRANS_STATE_WAITING_DISPOSE) {
                break;       /* 传输结束了，需要去处理传输结果 */
                i = NUM - 1; // 跳出For循环。
            }
        }
    }
    /* 这里hmodbusRTU->fAck03H == 0表示通信超时了，否侧写入03H参数成功 */
    status = hmodbusRTU->transEvent == MODBUS_RTU_TRANS_REV_ACK;
    modbus_rtu_trans_event_inner_deal(hmodbusRTU);
    MODBUS_RTU_RESET_TRANS_STATE(hmodbusRTU);
    return status;
}

/*
*********************************************************************************************************
*	函 数 名: modbus_rtu_host_read_04H
*	功能说明: 单个参数. 通过发送04H指令实现，发送之后，等待从机应答。
*	形    参: 无
*	返 回 值: 1 表示成功。0 表示失败（通信超时或被拒绝）
*********************************************************************************************************
*/
uint8_t modbus_rtu_host_read_04H(ModbusRTUInstance_t *hmodbusRTU, uint8_t slaveAddr, uint16_t _reg, uint16_t _num)
{

    uint8_t i;
    uint8_t status = 0; // 默认失败
    // 处于空闲状态
    if (hmodbusRTU->transStageState == MODBUS_RTU_TRANS_STATE_IDLE) {
        hmodbusRTU->transStageState = MODBUS_RTU_TRANS_STATE_TRANSMITING;
    } else {
        return status;
    }

    for (i = 0; i < NUM; i++) {
        MODH_Send04H(hmodbusRTU, slaveAddr, _reg, _num);
        hmodbusRTU->tickstart = HDL_G4_CPU_Time_GetTick(); /* 记录命令发送的时刻 */

        while (1) {
            modbus_rtu_handler();

            if (hmodbusRTU->transStageState == MODBUS_RTU_TRANS_STATE_WAITING_DISPOSE) {
                break;       /* 传输结束了，需要去处理传输结果 */
                i = NUM - 1; // 跳出For循环。
            }
        }
    }
    /* 这里hmodbusRTU->fAck04H == 0表示通信超时了，否侧写入04H参数成功 */
    status = hmodbusRTU->transEvent == MODBUS_RTU_TRANS_REV_ACK;
    modbus_rtu_trans_event_inner_deal(hmodbusRTU);
    MODBUS_RTU_RESET_TRANS_STATE(hmodbusRTU);
    return status;
}
/*
*********************************************************************************************************
*	函 数 名: modbus_rtu_host_write_05H
*	功能说明: 单个参数. 通过发送05H指令实现，发送之后，等待从机应答。
*	形    参: 无
*	返 回 值: 1 表示成功。0 表示失败（通信超时或被拒绝）
*********************************************************************************************************
*/
uint8_t modbus_rtu_host_write_05H(ModbusRTUInstance_t *hmodbusRTU, uint8_t slaveAddr, uint16_t _reg, uint16_t _value)
{

    uint8_t i;
    uint8_t status = 0; // 默认失败
    // 处于空闲状态
    if (hmodbusRTU->transStageState == MODBUS_RTU_TRANS_STATE_IDLE) {
        hmodbusRTU->transStageState = MODBUS_RTU_TRANS_STATE_TRANSMITING;
    } else {
        return status;
    }

    for (i = 0; i < NUM; i++) {
        MODH_Send05H(hmodbusRTU, slaveAddr, _reg, _value);
        hmodbusRTU->tickstart = HDL_G4_CPU_Time_GetTick(); /* 记录命令发送的时刻 */

        while (1) {
            modbus_rtu_handler();

            if (hmodbusRTU->transStageState == MODBUS_RTU_TRANS_STATE_WAITING_DISPOSE) {
                break;       /* 传输结束了，需要去处理传输结果 */
                i = NUM - 1; // 跳出For循环。
            }
        }
    }

    /* 这里hmodbusRTU->fAck05H == 0表示通信超时了，否侧写入05H参数成功 */
    status = hmodbusRTU->transEvent == MODBUS_RTU_TRANS_REV_ACK;
    modbus_rtu_trans_event_inner_deal(hmodbusRTU);
    MODBUS_RTU_RESET_TRANS_STATE(hmodbusRTU);
    return status;
}

/*
*********************************************************************************************************
*	函 数 名: modbus_rtu_host_write_06H
*	功能说明: 单个参数. 通过发送06H指令实现，发送之后，等待从机应答。循环NUM次写命令
*	形    参: 无
*	返 回 值: 1 表示成功。0 表示失败（通信超时或被拒绝）
*********************************************************************************************************
*/
uint8_t modbus_rtu_host_write_06H(ModbusRTUInstance_t *hmodbusRTU, uint8_t slaveAddr, uint16_t _reg, uint16_t _value)
{

    uint8_t i;
    uint8_t status = 0; // 默认失败
    // 处于空闲状态
    if (hmodbusRTU->transStageState == MODBUS_RTU_TRANS_STATE_IDLE) {
        hmodbusRTU->transStageState = MODBUS_RTU_TRANS_STATE_TRANSMITING;
    } else {
        return status;
    }

    for (i = 0; i < NUM; i++) {
        MODH_Send06H(hmodbusRTU, slaveAddr, _reg, _value);
        hmodbusRTU->tickstart = HDL_G4_CPU_Time_GetTick(); /* 记录命令发送的时刻 */

        while (1) {
            modbus_rtu_handler();

            if (hmodbusRTU->transStageState == MODBUS_RTU_TRANS_STATE_WAITING_DISPOSE) {
                break;       /* 传输结束了，需要去处理传输结果 */
                i = NUM - 1; // 跳出For循环。
            }
        }
    }

    /* 这里hmodbusRTU->fAck06H == 0表示通信超时了，否侧写入06H参数成功 */
    status = hmodbusRTU->transEvent == MODBUS_RTU_TRANS_REV_ACK;
    modbus_rtu_trans_event_inner_deal(hmodbusRTU);
    MODBUS_RTU_RESET_TRANS_STATE(hmodbusRTU);
    return status;
}

/*
*********************************************************************************************************
*	函 数 名: modbus_rtu_host_write_10H
*	功能说明: 单个参数. 通过发送10H指令实现，发送之后，等待从机应答。循环NUM次写命令
*	形    参: 无
*	返 回 值: 1 表示成功。0 表示失败（通信超时或被拒绝）
*********************************************************************************************************
*/
uint8_t modbus_rtu_host_write_10H(ModbusRTUInstance_t *hmodbusRTU, uint8_t slaveAddr, uint16_t _reg, uint8_t _num, uint8_t *_buf)
{

    uint8_t i;
    uint8_t status = 0; // 默认失败
    // 处于空闲状态
    if (hmodbusRTU->transStageState == MODBUS_RTU_TRANS_STATE_IDLE) {
        hmodbusRTU->transStageState = MODBUS_RTU_TRANS_STATE_TRANSMITING;
    } else {
        return status;
    }

    for (i = 0; i < NUM; i++) {
        MODH_Send10H(hmodbusRTU, slaveAddr, _reg, _num, _buf);
        hmodbusRTU->tickstart = HDL_G4_CPU_Time_GetTick(); /* 记录命令发送的时刻 */

        while (1) {
            modbus_rtu_handler();
            if (hmodbusRTU->transStageState == MODBUS_RTU_TRANS_STATE_WAITING_DISPOSE) {
                break;       /* 传输结束了，需要去处理传输结果 */
                i = NUM - 1; // 跳出For循环。
            }
        }
    }
    /* 这里hmodbusRTU->fAck10H == 0表示通信超时了，否侧写入10H参数成功 */
    status = hmodbusRTU->transEvent == MODBUS_RTU_TRANS_REV_ACK;
    modbus_rtu_trans_event_inner_deal(hmodbusRTU);
    MODBUS_RTU_RESET_TRANS_STATE(hmodbusRTU);
    return status;
}

/*
*********************************************************************************************************
*	函 数 名: RS485_SendBuf
*	功能说明: 通过RS485芯片发送一串数据。注意，本函数不等待发送完毕。
*	形    参: _ucaBuf : 数据缓冲区
*			  _usLen : 数据长度
*	返 回 值: 无
*********************************************************************************************************
*/
void RS485_SendBuf(ModbusRTUInstance_t *hmodbusRTU, uint8_t *_ucaBuf, uint16_t _usLen)
{
    // get_bus成员函数肯定已经初始化了才能使用。
    hmodbusRTU->get_bus();
    Uart_Write(hmodbusRTU->com, _ucaBuf, _usLen);
}

/*
*********************************************************************************************************
*	函 数 名: modbus_rtu_handler
*	功能说明: 空闲时执行的函数。一般主程序在for和while循环程序体中需要插入 CPU_IDLE() 宏来调用本函数。
*			 本函数缺省为空操作。用户可以添加喂狗、设置CPU进入休眠模式的功能。
*	形    参: 无
*	返 回 值: 无
*********************************************************************************************************
*/
void modbus_rtu_handler(void)
{
/* --- 喂狗 */

/* --- 让CPU进入休眠，由Systick定时中断唤醒或者其他中断唤醒 */
#if USEING_POLL_MODE_TO_GET_CHAR_STREAM == 1
    modbus_rtu_host_uart_msg_fatch();
#endif // USEING_POLL_MODE_TO_GET_CHAR_STREAM

    modbus_rtu_poll();
    modbus_rtu_timeout_handler();
    modbus_rtu_trans_event_inner_handler();
}

/**
 * @brief 初始化Modbus RTU实例。
 *
 * @param hmodbusRTU RTU实例句柄，可选hModbusRTU3~hModbusRTU5
 * @param baud 通信波特率，eg. 9600, 19200, 57600, 115200, etc.
 * @param parity 'N' for none, 'E' for even, 'O' for odd
 * @param data_bit 串口通信的位宽，可以是7, 8 and 9.
 */
void modbus_rtu_host_init(ModbusRTUInstance_t *hmodbusRTU, int baud, char parity, int data_bit)
{
    hmodbusRTU->TIMEOUT  = 30;
    hmodbusRTU->_TxCount = 0;
    hmodbusRTU->_TxCount = 0;
    uint32_t wordLen     = LL_USART_DATAWIDTH_8B;
    uint32_t parity_     = LL_USART_PARITY_NONE;
    uint32_t stopBit     = LL_USART_STOPBITS_1;
    wordLen              = data_bit == 7 ? LL_USART_DATAWIDTH_7B : LL_USART_DATAWIDTH_8B;
    wordLen              = data_bit == 9 ? LL_USART_DATAWIDTH_9B : LL_USART_DATAWIDTH_8B;
    parity_              = parity == 'E' ? LL_USART_PARITY_EVEN : LL_USART_PARITY_NONE;
    parity_              = parity == 'O' ? LL_USART_PARITY_ODD : LL_USART_PARITY_NONE;

    stopBit          = parity_ == LL_USART_PARITY_NONE ? LL_USART_STOPBITS_2 : LL_USART_STOPBITS_1;
    hmodbusRTU->baud = baud;

    /* 根据波特率，获取需要延迟的时间 */
    hmodbusRTU->usTimeOut35       = 1000000.0f / baud * 11.0f * 3.5f + 0.5f; // 1秒的微秒数/波特率*11bit的帧长度*3.5字符+0.5f四舍五入
    hmodbusRTU->usTimeOut15       = 1000000.0f / baud * 11.0f * 1.5f + 0.5f; // 1秒的微秒数/波特率*11bit的帧长度*3.5字符+0.5f四舍五入
    hmodbusRTU->lastRevByteUsTick = 0;

    hmodbusRTU->gpio_init();
    Uart_SetWriteOverCallback(hmodbusRTU->com, hmodbusRTU->release_bus);
#if USEING_POLL_MODE_TO_GET_CHAR_STREAM == 0
    Uart_RegisterReceiveCharCallback(hmodbusRTU->com, hmodbusRTU->receive_char_callback);
#endif // USEING_POLL_MODE_TO_GET_CHAR_STREAM

    Uart_Init(hmodbusRTU->com, baud, wordLen, parity_, stopBit);
    hmodbusRTU->innerTimeoutEventHandler = NULL;
    hmodbusRTU->enabled                  = 1;
}

/**
 * @brief 将串口消息队列中的数据传递到modbus host处理方法
 *
 */
static void modbus_rtu_host_uart_msg_fatch()
{
    static uint32_t data_len        = 0;
    static uint8_t buf              = 0;
    ModbusRTUInstance_t *hmodbusRTU = NULL;
    for (size_t i = 0; i < MODBUS_RTUS_HOST_NUM; i++) {
        hmodbusRTU = &modbusRTUList[i];
        // Modbus RTU host未使能
        if (!hmodbusRTU->enabled) {
            continue;
        }

        // 实现t3.5超时处理，没有实现字符间隔超过t1.5间隔的异常捕获功能，理论上接收到笑一个字符的时间间隔
        // 大于t1.5小于t3.5判定为异常，不过要保证计时精度足够，这里没有实现。
        // 实际要实现t1.5超时建议是大于t1.6小于t3.0
        //  && (hmodbusRTU->RxCount > 0)收到了一个字节数据才开始检查超时。
        if (((HDL_G4_CPU_Time_GetUsTick() - hmodbusRTU->lastRevByteUsTick) > hmodbusRTU->usTimeOut35) && (hmodbusRTU->_RxCount > 0)) {
            MODH_RxTimeOut(hmodbusRTU);
        }

        data_len = Uart_Read(hmodbusRTU->com, &buf, 1);
        if (data_len > 0) {
            MODH_ReciveNew(hmodbusRTU, buf);
        }
    }
}

/**
 * @brief 功能与modbus_rtu_host_read_param_01H类似，不同之处在于该方法只会发送01H命令。
 * 而且如果一个命令还没有收到回复，也就是上一个传输未完成那么该方法会返回0表示发送失败。
 * 该方法不会检查Modbus RTU是否初始化了。
 *
 * @param hmodbusRTU
 * @param slaveAddr 从机地址
 * @param _reg 从机寄存器地址
 * @param _num
 * @return uint8_t 1 表示成功。0 表示失败（上一个传输未完）
 */
uint8_t modbus_rtu_host_read_cmd_01H(ModbusRTUInstance_t *hmodbusRTU, uint8_t slaveAddr, uint16_t _reg, uint16_t _num)
{
    uint8_t status = 0;
    // 处于空闲状态
    if (hmodbusRTU->transStageState == MODBUS_RTU_TRANS_STATE_IDLE) {
        hmodbusRTU->transStageState = MODBUS_RTU_TRANS_STATE_TRANSMITING;
        MODH_Send01H(hmodbusRTU, slaveAddr, _reg, _num);   /* 发送命令 */
        hmodbusRTU->tickstart = HDL_G4_CPU_Time_GetTick(); /* 记录命令发送的时刻 */
        status                = 1;
    }

    return status;
}

uint8_t modbus_rtu_host_read_cmd_02H(ModbusRTUInstance_t *hmodbusRTU, uint8_t slaveAddr, uint16_t _reg, uint16_t _num)
{
    uint8_t status = 0;
    // 处于空闲状态
    if (hmodbusRTU->transStageState == MODBUS_RTU_TRANS_STATE_IDLE) {
        hmodbusRTU->transStageState = MODBUS_RTU_TRANS_STATE_TRANSMITING;
        MODH_Send01H(hmodbusRTU, slaveAddr, _reg, _num);   /* 发送命令 */
        hmodbusRTU->tickstart = HDL_G4_CPU_Time_GetTick(); /* 记录命令的发送时刻 */
        status                = 1;
    }

    return status;
}
uint8_t modbus_rtu_host_read_cmd_03H(ModbusRTUInstance_t *hmodbusRTU, uint8_t slaveAddr, uint16_t _reg, uint16_t _num)
{
    uint8_t status = 0;
    // 处于空闲状态
    if (hmodbusRTU->transStageState == MODBUS_RTU_TRANS_STATE_IDLE) {
        hmodbusRTU->transStageState = MODBUS_RTU_TRANS_STATE_TRANSMITING;
        MODH_Send03H(hmodbusRTU, slaveAddr, _reg, _num);   /* 发送命令 */
        hmodbusRTU->tickstart = HDL_G4_CPU_Time_GetTick(); /* 记录命令的发送时刻 */
        status                = 1;
    }

    return status;
}
uint8_t modbus_rtu_host_read_cmd_04H(ModbusRTUInstance_t *hmodbusRTU, uint8_t slaveAddr, uint16_t _reg, uint16_t _num)
{
    uint8_t status = 0;
    // 处于空闲状态
    if (hmodbusRTU->transStageState == MODBUS_RTU_TRANS_STATE_IDLE) {
        hmodbusRTU->transStageState = MODBUS_RTU_TRANS_STATE_TRANSMITING;
        MODH_Send04H(hmodbusRTU, slaveAddr, _reg, _num);   /* 发送命令 */
        hmodbusRTU->tickstart = HDL_G4_CPU_Time_GetTick(); /* 记录命令的发送时刻 */
        status                = 1;
    }

    return status;
}
uint8_t modbus_rtu_host_write_cmd_05H(ModbusRTUInstance_t *hmodbusRTU, uint8_t slaveAddr, uint16_t _reg, uint16_t _value)
{
    uint8_t status = 0;
    // 处于空闲状态
    if (hmodbusRTU->transStageState == MODBUS_RTU_TRANS_STATE_IDLE) {
        hmodbusRTU->transStageState = MODBUS_RTU_TRANS_STATE_TRANSMITING;
        MODH_Send05H(hmodbusRTU, slaveAddr, _reg, _value); /* 发送命令 */
        hmodbusRTU->tickstart = HDL_G4_CPU_Time_GetTick(); /* 记录命令的发送时刻 */
        status                = 1;
    }

    return status;
}
uint8_t modbus_rtu_host_write_cmd_06H(ModbusRTUInstance_t *hmodbusRTU, uint8_t slaveAddr, uint16_t _reg, uint16_t _value)
{
    uint8_t status = 0;
    // 处于空闲状态
    if (hmodbusRTU->transStageState == MODBUS_RTU_TRANS_STATE_IDLE) {
        hmodbusRTU->transStageState = MODBUS_RTU_TRANS_STATE_TRANSMITING;
        MODH_Send06H(hmodbusRTU, slaveAddr, _reg, _value); /* 发送命令 */
        hmodbusRTU->tickstart = HDL_G4_CPU_Time_GetTick(); /* 记录命令的发送时刻 */
        status                = 1;
    }

    return status;
}
uint8_t modbus_rtu_host_write_cmd_10H(ModbusRTUInstance_t *hmodbusRTU, uint8_t slaveAddr, uint16_t _reg, uint8_t _num, uint8_t *_buf)
{
    uint8_t status = 0;
    // 处于空闲状态
    if (hmodbusRTU->transStageState == MODBUS_RTU_TRANS_STATE_IDLE) {
        hmodbusRTU->transStageState = MODBUS_RTU_TRANS_STATE_TRANSMITING;
        MODH_Send10H(hmodbusRTU, slaveAddr, _reg, _num, _buf); /* 发送命令 */
        hmodbusRTU->tickstart = HDL_G4_CPU_Time_GetTick();     /* 记录命令的发送时刻 */
        status                = 1;
    }
    return status;
}

/**
 * @brief 查询是否有Modbus RTU命令回复事件，也就是传输处理骤需要处理的结果。这个方法需要用户自行处理超时结果。
 *
 * @param hmodbusRTU
 * @return uint8_t 0表示没有事件需要处理，1表示有事件需要处理。
 */
inline uint8_t modbus_rtu_host_have_trans_event(ModbusRTUInstance_t *hmodbusRTU)
{
    return hmodbusRTU->transStageState == MODBUS_RTU_TRANS_STATE_WAITING_DISPOSE;
}

/**
 * @brief 匹配回复事件，根据事件、本次传输的命令的功能码。
 *
 * @param hmodbusRTU
 * @param event 事件。如果是MODBUS_RTU_TRANS_ANY_EVENT，那么就只判断是否有事件，而不管后面的条件。
 * @param cmd 功能码。
 * @param slave_addr 这次事件对于的目标从设备的地址。如果是MODBUS_RTU_ANY_ADDRESS，表示匹配任何地址。
 * @return uint8_t 1匹配成功，0匹配失败。
 */
uint8_t modbus_rtu_host_match_event(ModbusRTUInstance_t *hmodbusRTU, ModbusRTU_TransEvent_t event, uint8_t slave_addr, uint8_t func_code)
{
    // 没有事件
    if (hmodbusRTU->transStageState != MODBUS_RTU_TRANS_STATE_WAITING_DISPOSE) {
        return 0;
    }

    // 匹配所有事件
    if (event == MODBUS_RTU_TRANS_ANY_EVENT) {
        return (hmodbusRTU->transEvent != MODBUS_RTU_TRANS_RES_NONE);
    }

    if (slave_addr == MODBUS_RTU_ANY_ADDRESS) {
        return (hmodbusRTU->transEvent == event) &&
               (hmodbusRTU->respons_func_code == func_code);
    }

    return (hmodbusRTU->transEvent == event) &&
           (hmodbusRTU->respons_func_code == func_code) &&
           (hmodbusRTU->slaveAddr == slave_addr);
}

/**
 * @brief 重置Modbus的传输步骤状态，转化为空闲状态。在每次在每次modbus_rtu_host_have_trans_event_xxH
 * 查询到有事件后处理完事件执行。不执行的话下一次命令无法发送。
 * 负责完成传输的重置和传输超时处理。从事件角度来说就说相当于清除所有事件。
 * 其实应该起名为clear all event.
 *
 * @param hmodbusRTU
 */
inline void modbus_rtu_host_clear_all_trans_event(ModbusRTUInstance_t *hmodbusRTU)
{
    if (modbus_rtu_host_have_trans_event(hmodbusRTU)) {
        // modbus_rtu_trans_event_inner_deal(hmodbusRTU);
        // 如果不在这里清除那就只有在发送时清除，这样可以实现查询到事件后不处理就清除，在某些情况下有用
        // 例如可以保证完全能够控制处理完数据后再发新的命令的情况，但是这里先禁止这种情况。
        hmodbusRTU->transEvent = MODBUS_RTU_TRANS_RES_NONE;
        MODBUS_RTU_RESET_TRANS_STATE(hmodbusRTU);
    }
}

/**
 * @brief 查询Modbus RTU命令回复结果，也就是传输处理骤需要处理的结果。这个方法本身会处理超时结果，
 * 超时结果将不会作为一个事件反应到用户。
 *
 * @param hmodbusRTU
 * @return uint8_t 0表示没有事件需要处理，1表示需要处理收到的回复。
 */
uint8_t modbus_rtu_host_query_trans_result(ModbusRTUInstance_t *hmodbusRTU)
{
    uint8_t status = 0;
    if (hmodbusRTU->transStageState == MODBUS_RTU_TRANS_STATE_WAITING_DISPOSE) {
        // 在内部就处理掉一些列错误事件，例如超时，这样就不需要用户操心超时的问题了。
        modbus_rtu_trans_event_inner_deal(hmodbusRTU);
        status = hmodbusRTU->transEvent == MODBUS_RTU_TRANS_REV_ACK;
    }
    return status;
}

/**
 * @brief 从modbus rtu收到的数据中按照uint16_t格式读取num个uint16_t数据。
 *
 * @param hmodbusRTU
 * @param pBuf 存放读取数据的缓存的指针。
 * @param Num 返回实际实际读取了多少个uint16_t类型的数据。
 */
int modbus_rtu_get_data_uint16(ModbusRTUInstance_t *hmodbusRTU, uint16_t *pBuf, int Num)
{
    int read_len = 0; // 实际要读取几个指定类型的数据
    // 这里假定了收到的数据是正常的Modbus帧
    read_len      = hmodbusRTU->RxBuf[2] / sizeof(uint16_t); /* 数据长度 字节数 */
    read_len      = read_len > Num ? Num : read_len;
    uint8_t *data = &hmodbusRTU->RxBuf[3];

    for (size_t i = 0; i < read_len; i++) {
        pBuf[i] = ((uint16_t)data[0] << 8) | ((uint16_t)data[1]);
        data += sizeof(uint16_t);
    }

    return read_len;
}

/**
 * @brief 从modbus rtu收到的数据中按照int16_t格式读取num个int16_t数据。
 *
 * @param hmodbusRTU
 * @param pBuf 存放读取数据的缓存的指针。
 * @param Num 返回实际实际读取了多少个int16类型的数据。
 */
int modbus_rtu_get_data_int16(ModbusRTUInstance_t *hmodbusRTU, int16_t *pBuf, int Num)
{
    int read_len = 0; // 实际要读取几个指定类型的数据
    // 这里假定了收到的数据是正常的Modbus帧
    read_len      = hmodbusRTU->RxBuf[2] / sizeof(int16_t); /* 数据长度 字节数 */
    read_len      = read_len > Num ? Num : read_len;
    uint8_t *data = &hmodbusRTU->RxBuf[3];
    uint16_t tmp  = 0;
    for (size_t i = 0; i < read_len; i++) {
        tmp     = ((uint16_t)data[0] << 8) | ((uint16_t)data[1]);
        pBuf[i] = *((int16_t *)(&tmp));
        data += sizeof(int16_t);
    }

    return read_len;
}