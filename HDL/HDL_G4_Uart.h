/**
 * @file HDL_G4_Uart.h
 * @author Liu Yuanlin (liuyuanlins@outlook.com)
 * @brief change from HDL_USR_Uart_L476.h
 * @version 0.1
 * @date 2022-10-04
 *
 * @copyright Copyright (c) 2022 Liu Yuanlin Personal.
 *
 */
/*
 * HDL_USR_Uart_L476.h
 *
 *  Created on: 2019年3月12日
 *      Author: WangJianping
 *      抽象所有串口读写操作
 *      写操作-直接写串口
 *      读操作-从队列中直接读，队列大小1K
 *      初始化操作--在使能相关串口初始化参数,队列初始化
 *
 */

#ifndef HDL_USR_UART_H_
#define HDL_USR_UART_H_
#ifdef __cplusplus
extern "C"
{
#endif
#include "main.h"

    // 串口号定义
    typedef enum
    {
        COM1 = 1,
        COM2 = 2,
        COM3 = 3,
        COM4 = 4,
        COM5 = 5,
        COM6 = 6,
    } COMID_t;

    typedef void (*UartWriteOverCallback_t)(void);
    typedef void (*UartReceiveCharCallback_t)(uint8_t ch);

    typedef struct tagCOM_Dev_t
    {
        COMID_t com;
        // 主要是给Modbus使用，因为使用FIFO，发送完成时机不等于释放总线的时机。
        // 其时机是连续发送一串，然后直到硬件FIFO中的数据完全发送出去了产生回调，回调属于中断函数。
        // 这里实现依赖于串口的TC中断。
        // 这意味着如果两个Uart_Write对同一个串口写入的时机过于接近，那么就无法使得一次Uart_Write
        // 产生一个回调。
        UartWriteOverCallback_t write_over_callback;
        UartReceiveCharCallback_t receive_char_callback;
        void (*receive_char_callback_not_read)(void);
        // 串口是否初始化
        uint8_t inited;
    } COM_Dev_t;

    void Uart_Init(COMID_t comid, uint32_t bound, uint32_t wordLen, uint32_t stopBit, uint32_t parity);
    uint32_t Uart_Write(COMID_t comid, uint8_t *writeBuf, uint32_t uLen);
    uint32_t Uart_Read(COMID_t comid, uint8_t *pBuf, uint32_t uiLen);
    uint8_t Uart_SetWriteOverCallback(COMID_t comid, UartWriteOverCallback_t callback);
    uint8_t Uart_RegisterReceiveCharCallback(COMID_t comid, UartReceiveCharCallback_t callback);
    uint8_t Uart_RegisterReceiveCharNotReadCallback(COMID_t comid, void(* callback)(void));
    uint8_t Uart_UnregisterReceiveCharCallback(COMID_t comid);

#ifdef __cplusplus
}
#endif
#endif /* HDL_USR_UART_H_ */
