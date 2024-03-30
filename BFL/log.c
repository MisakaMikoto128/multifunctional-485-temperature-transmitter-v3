#include "log.h"
#include <stdarg.h>
#include "HDL_G4_CPU_Time.h"
#include "HDL_G4_Uart.h"
#include <stdio.h>

#undef FREE_RTOS

#ifdef FREE_RTOS
#include "FreeRTOS.h"
#include "task.h"
#endif // FREE_RTOS

#define USING_RTT     0
#define USING_USB_CDC 1
#define USING_UART    0
#if USING_RTT == 1
#include "SEGGER_RTT.h"
#elif USING_USB_CDC == 1
#include "usbd_cdc_if.h"
#endif

// 调试输出数据包最大长度
#define MAXDEBUGSEND 256
static uint8_t buffer[MAXDEBUGSEND + 1];
#define DEBUG_COM COM2

/*
    Refference:
    mtime_t mtime;
    uint64_t timestamp = HDL_RTC_GetTimeTick(&mtime.wSub);
    timestamp += 8 * 3600;
    mtime_utc_sec_2_time(timestamp, &mtime);
    SEGGER_RTT_printf(0, "%04d.%02d.%02d %02d:%02d:%02d  [%s]: %s\n",
                        mtime.nYear, mtime.nMonth, mtime.nDay, mtime.nHour, mtime.nMin, mtime.nSec,
                        ulog_level_name(severity),
                        msg);
*/

/**
 * @brief 使用串口调试的格式化输出方法
 *
 * @param format
 * @param ...
 */
void Debug_Printf(const void *format, ...)
{
    uint32_t uLen;
    va_list vArgs;
    va_start(vArgs, format);
    uLen = vsnprintf((char *)buffer, MAXDEBUGSEND, (char const *)format, vArgs);
    va_end(vArgs);
    if (uLen > MAXDEBUGSEND)
        uLen = MAXDEBUGSEND;
#if USING_RTT == 1
    SEGGER_RTT_Write(0, buffer, uLen);
#elif USING_USB_CDC == 1
    CDC_Transmit_FS(buffer, uLen);
#elif USING_UART == 1
    Uart_Write(DEBUG_COM, buffer, uLen);
#endif
}

void my_console_logger(ulog_level_t severity, char *msg)
{
#ifdef ULOG_ENABLED
#ifdef FREE_RTOS
    taskENTER_CRITICAL();
#endif // FREE_RTOS
    Debug_Printf("%u [%s]: %s\n",
                 HDL_G4_CPU_Time_GetTick(),
                 ulog_level_name(severity),
                 msg);
#ifdef FREE_RTOS
    taskEXIT_CRITICAL();
#endif // FREE_RTOS
#endif // ULOG_ENABLED
}

void ulog_init_user()
{
    ULOG_INIT();

#if USING_RTT == 1
    /* 配置通道 0，上行配置*/
    SEGGER_RTT_ConfigUpBuffer(0, "RTTUP", NULL, 0, SEGGER_RTT_MODE_NO_BLOCK_TRIM);
    /* 配置通道 0，下行配置*/
    SEGGER_RTT_ConfigDownBuffer(0, "RTTDOWN", NULL, 0, SEGGER_RTT_MODE_NO_BLOCK_TRIM);
    SEGGER_RTT_SetTerminal(0);
#elif USING_USB_CDC == 1
#else
    Uart_Init(DEBUG_COM, 1500000, LL_USART_DATAWIDTH_8B, LL_USART_STOPBITS_1, LL_USART_PARITY_NONE);
#endif
    // dynamically change the threshold for a specific logger
    ULOG_SUBSCRIBE(my_console_logger, ULOG_DEBUG_LEVEL);
}