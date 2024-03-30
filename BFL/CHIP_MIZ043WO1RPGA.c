/**
 * @file CHIP_MIZ043WO1RPGA.c
 * @author Liu Yuanlin (liuyuanlins@outlook.com)
 * @brief
 * @version 0.1
 * @date 2024-03-28
 * @last modified 2024-03-28
 *
 * @copyright Copyright (c) 2024 Liu Yuanlin Personal.
 *
 */
#include "CHIP_MIZ043WO1RPGA.h"
#include "HDL_G4_Uart.h"
#include "asyn_sys.h"
#include "HDL_G4_CPU_Time.h"
#include <stdarg.h>
#include <string.h>
#include <stdio.h>
#include "crc.h"
#define __MIZ043WO1RPGA_COM                    COM1

#define CHIP_MIZ043WO1RPGA_FRAME_INTERVAL_TIME 15 // 单位：ms
#define __MIZ043WO1RPGA_MODBUS_RES_LEN         9

struct MIZ043WO1RPGA_Msg {
    uint8_t buffer[CHIP_MIZ043WO1RPGA_WIDGETS_MAX_LEN + __MIZ043WO1RPGA_MODBUS_RES_LEN];
    uint8_t length;
};

#define __MIZ043WO1RPGA_MSGQUEUE_CAPACITY 8
struct MIZ043WO1RPGA_MsgBuffer {
    struct MIZ043WO1RPGA_Msg msg_buf[__MIZ043WO1RPGA_MSGQUEUE_CAPACITY];
    int head;
    int tail;
    int capacity;
};

static struct MIZ043WO1RPGA_MsgBuffer s_tMsgBuffer = {
    .head     = 0,
    .tail     = 0,
    .capacity = __MIZ043WO1RPGA_MSGQUEUE_CAPACITY,
    .msg_buf  = {0},
};

static void CHIP_MIZ043WO1RPGA_Poll()
{
    static PeriodREC_t s_tPollTime = 0;
    if (period_query_user(&s_tPollTime, CHIP_MIZ043WO1RPGA_FRAME_INTERVAL_TIME)) {
        // 4. 发送数据
        if (s_tMsgBuffer.head != s_tMsgBuffer.tail) {
            Uart_Write(__MIZ043WO1RPGA_COM, s_tMsgBuffer.msg_buf[s_tMsgBuffer.head].buffer, s_tMsgBuffer.msg_buf[s_tMsgBuffer.head].length);
            s_tMsgBuffer.head = (s_tMsgBuffer.head + 1) % s_tMsgBuffer.capacity;
        }
    }
}

void CHIP_MIZ043WO1RPGA_Init()
{
    // 初始化串口
    Uart_Init(__MIZ043WO1RPGA_COM, 115200, LL_USART_DATAWIDTH_8B, LL_USART_STOPBITS_1, LL_USART_PARITY_NONE);

    asyn_sys_register(CHIP_MIZ043WO1RPGA_Poll); // 注册回调函数
}

/**
 * @brief 
 * 
 * @param widgetAddress 空间的地址，参考界面设计软件
 * @param widgetMaxLength 空间的最大长度，最大为@CHIP_MIZ043WO1RPGA_WIDGETS_MAX_LEN
 * @param fmt 
 * @param ... 
 */
void CHIP_MIZ043WO1RPGA_Printf(int widgetAddress, int widgetMaxLength, const char *fmt, ...)
{

    if (widgetMaxLength > CHIP_MIZ043WO1RPGA_WIDGETS_MAX_LEN) {
        widgetMaxLength = CHIP_MIZ043WO1RPGA_WIDGETS_MAX_LEN;
    }

    uint32_t uLen;
    va_list vArgs;
    uint8_t buffer[CHIP_MIZ043WO1RPGA_WIDGETS_MAX_LEN + 1];

    va_start(vArgs, fmt);
    uLen = vsnprintf((char *)buffer, widgetMaxLength + 1, (char const *)fmt, vArgs);
    uLen = uLen > 0 ? uLen - 1 : 0;
    va_end(vArgs);
    if (uLen > widgetMaxLength)
        uLen = widgetMaxLength;

    // 默认左对齐

    uint8_t modbus_buffer[CHIP_MIZ043WO1RPGA_WIDGETS_MAX_LEN + __MIZ043WO1RPGA_MODBUS_RES_LEN];

    // 编码数据，使用Modbus协议，指令码为0x10
    modbus_buffer[0] = 0x00;                        // 从机地址
    modbus_buffer[1] = 0x10;                        // 功能码
    modbus_buffer[2] = (widgetAddress >> 8) & 0xFF; // 寄存器地址高位
    modbus_buffer[3] = widgetAddress & 0xFF;        // 寄存器地址低位
    modbus_buffer[4] = 0x00;                        // 寄存器数量高位
    modbus_buffer[5] = (uLen + 1) / 2;              // 寄存器数量低位，向上取整
    modbus_buffer[6] = modbus_buffer[5] * 2;        // 数据字节数
    for (size_t i = 0; i < modbus_buffer[6]; i++) {
        modbus_buffer[7 + i * 2] = buffer[i * 2 + 1];
        modbus_buffer[8 + i * 2] = buffer[i * 2];
    }

    // 计算CRC16校验码
    uint16_t crc;

    crc = CRC16_Modbus(modbus_buffer, 7 + modbus_buffer[6]);

    modbus_buffer[7 + modbus_buffer[6]] = (crc >> 8) & 0xFF;
    modbus_buffer[8 + modbus_buffer[6]] = crc & 0xFF;

    // 存入发送缓冲区
    // 1. 判断缓冲区是否已满
    if ((s_tMsgBuffer.tail + 1) % s_tMsgBuffer.capacity == s_tMsgBuffer.head) {
        // 缓冲区已满
        return;
    }

    // 2. 存入缓冲区
    s_tMsgBuffer.msg_buf[s_tMsgBuffer.tail].length = 9 + modbus_buffer[6];
    memcpy(s_tMsgBuffer.msg_buf[s_tMsgBuffer.tail].buffer, modbus_buffer, 9 + modbus_buffer[6]);

    // 3. 更新尾指针
    s_tMsgBuffer.tail = (s_tMsgBuffer.tail + 1) % s_tMsgBuffer.capacity;
}