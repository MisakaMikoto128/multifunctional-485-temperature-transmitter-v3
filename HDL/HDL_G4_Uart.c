/**
 * @file HDL_G4_Uart.c
 * @author Liu Yuanlin (liuyuanlins@outlook.com)
 * @brief
 * @version 0.1
 * @date 2022-10-04
 *
 * @copyright Copyright (c) 2022 Liu Yuanlin Personal.
 *
 */

#include "HDL_G4_Uart.h"
#include "mQueue.h"
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
/* 开关全局中断的宏 */
#define ENABLE_INT()  __set_PRIMASK(0) /* 使能全局中断 */
#define DISABLE_INT() __set_PRIMASK(1) /* 禁止全局中断 */

/**************************
 *
 *
优先级：
NVIC_PRIORITYGROUP_4 :
    4 bits for pre-emption priority,
    0 bit  for subpriority

COM map:
COM1 <---> USART1
COM2 <---> USART2
COM3 <---> USART3
COM4 <---> UART4
COM5 <---> UART5
COM6 <---> LPUART1

PIN Map:

USART1
    PA9   ------> USART1_TX
    PA10   ------> USART1_RX
USART2
    PA15   ------> USART2_RX
    PB3   ------> USART2_TX
USART3
    PB8-BOOT0   ------> USART3_RX
    PB9   ------> USART3_TX
UART4
    PC10   ------> UART4_TX
    PC11   ------> UART4_RX
UART5
    PC12   ------> UART5_TX
    PD2   ------> UART5_RX
LPUART1
    PB10   ------> LPUART1_RX
    PB11   ------> LPUART1_TX


G473:
USART1
    PA9   ------> USART1_TX
    PA10  ------> USART1_RX
USART2
    PD6   ------> USART2_RX
    PD5   ------> USART2_TX
USART3
    PD9   ------> USART3_RX
    PD8   ------> USART3_TX
UART4
    PC10  ------> UART4_TX
    PC11  ------> UART4_RX
UART5
    PD2  ------> UART5_TX
    PC12   ------> UART5_RX
LPUART1
    PC0   ------> LPUART1_RX
    PC1   ------> LPUART1_TX

    这个串口库的发送全部启用了FIFO，大小为8，并且启用了TX FIFO empty中断。
    如果对字符实时性要求高的场合FIFO可能会产生不好的影响，需要自己修改。
**************************/

// 串口设备抽象
COM_Dev_t COM1_Dev = {.com = COM1, .inited = 0, .write_over_callback = NULL, .receive_char_callback = NULL};
COM_Dev_t COM2_Dev = {.com = COM2, .inited = 0, .write_over_callback = NULL, .receive_char_callback = NULL};
COM_Dev_t COM3_Dev = {.com = COM3, .inited = 0, .write_over_callback = NULL, .receive_char_callback = NULL};
COM_Dev_t COM4_Dev = {.com = COM4, .inited = 0, .write_over_callback = NULL, .receive_char_callback = NULL};
COM_Dev_t COM5_Dev = {.com = COM5, .inited = 0, .write_over_callback = NULL, .receive_char_callback = NULL};
COM_Dev_t COM6_Dev = {.com = COM6, .inited = 0, .write_over_callback = NULL, .receive_char_callback = NULL};

// 串口1相关变量
#define COM1_RX_BUF_SIZE 400
static QUEUE8_t m_QueueCom1Rx                = {0};
static uint8_t m_Com1RxBuf[COM1_RX_BUF_SIZE] = {0};

#define COM1_TX_BUF_SIZE 400
static QUEUE8_t m_QueueCom1Tx                = {0};
static uint8_t m_Com1TxBuf[COM1_TX_BUF_SIZE] = {0};

// 串口2相关变量
#define COM2_RX_BUF_SIZE 400
static QUEUE8_t m_QueueCom2Rx                = {0};
static uint8_t m_Com2RxBuf[COM2_RX_BUF_SIZE] = {0};

#define COM2_TX_BUF_SIZE 400
static QUEUE8_t m_QueueCom2Tx                = {0};
static uint8_t m_Com2TxBuf[COM2_TX_BUF_SIZE] = {0};

// 串口3相关变量
#define COM3_RX_BUF_SIZE 100
static QUEUE8_t m_QueueCom3Rx                = {0};
static uint8_t m_Com3RxBuf[COM3_RX_BUF_SIZE] = {0};
#define COM3_TX_BUF_SIZE 100
static QUEUE8_t m_QueueCom3Tx                = {0};
static uint8_t m_Com3TxBuf[COM3_TX_BUF_SIZE] = {0};

// 串口4相关变量
#define COM4_RX_BUF_SIZE 100
static QUEUE8_t m_QueueCom4Rx                = {0};
static uint8_t m_Com4RxBuf[COM4_RX_BUF_SIZE] = {0};

// 串口5相关变量
#define COM5_RX_BUF_SIZE 100
static QUEUE8_t m_QueueCom5Rx                = {0};
static uint8_t m_Com5RxBuf[COM5_RX_BUF_SIZE] = {0};

// 串口6相关变量
#define COM6_RX_BUF_SIZE 100
static QUEUE8_t m_QueueCom6Rx                = {0};
static uint8_t m_Com6RxBuf[COM6_RX_BUF_SIZE] = {0};

/**
 * @brief 串口初始化
 *
 * @param comx 串口号
 * @param bound 波特率
 * @param wordLen 数据宽度 LL_USART_DATAWIDTH_7B,LL_USART_DATAWIDTH_8B,LL_USART_DATAWIDTH_9B
 * @param stopBit 停止位个数LL_USART_STOPBITS_1,LL_USART_STOPBITS_2
 * @param parity 奇偶校验位LL_USART_PARITY_NONE LL_USART_PARITY_EVEN LL_USART_PARITY_ODD
 * @note 如果是串口是LPUART，那么推荐使用的初始化参数名字如下：
 * wordLen-LL_LPUART_DATAWIDTH_7B,LL_LPUART_DATAWIDTH_8B,LL_LPUART_DATAWIDTH_9B
 * stopBit-LL_LPUART_STOPBITS_1,LL_LPUART_STOPBITS_2
 * parity-LL_LPUART_PARITY_NONE,LL_LPUART_PARITY_EVEN,LL_LPUART_PARITY_ODD
 */
void Uart_Init(COMID_t comid, uint32_t bound, uint32_t wordLen, uint32_t stopBit,
               uint32_t parity)
{
    LL_GPIO_InitTypeDef GPIO_InitStruct    = {0};
    LL_USART_InitTypeDef USART_InitStruct  = {0};
    RCC_PeriphCLKInitTypeDef PeriphClkInit = {0};

    switch (comid) {
        case COM1: {
            // 创建接收数据缓冲区
            QUEUE_PacketCreate(&m_QueueCom1Rx, m_Com1RxBuf, sizeof(m_Com1RxBuf));
            // 创建发送数据缓冲区
            QUEUE_PacketCreate(&m_QueueCom1Tx, m_Com1TxBuf, sizeof(m_Com1TxBuf));
            LL_RCC_SetUSARTClockSource(LL_RCC_USART1_CLKSOURCE_PCLK2);
            LL_APB2_GRP1_EnableClock(LL_APB2_GRP1_PERIPH_USART1);
            LL_AHB2_GRP1_EnableClock(LL_AHB2_GRP1_PERIPH_GPIOA);
            /**USART1 GPIO Configuration
             PA9   ------> USART1_TX
             PA10   ------> USART1_RX
             */
            GPIO_InitStruct.Pin        = LL_GPIO_PIN_9 | LL_GPIO_PIN_10;
            GPIO_InitStruct.Mode       = LL_GPIO_MODE_ALTERNATE;
            GPIO_InitStruct.Speed      = LL_GPIO_SPEED_FREQ_VERY_HIGH;
            GPIO_InitStruct.OutputType = LL_GPIO_OUTPUT_PUSHPULL;
            GPIO_InitStruct.Pull       = LL_GPIO_PULL_NO;
            GPIO_InitStruct.Alternate  = LL_GPIO_AF_7;
            LL_GPIO_Init(GPIOA, &GPIO_InitStruct);
            NVIC_SetPriority(USART1_IRQn,
                             NVIC_EncodePriority(NVIC_GetPriorityGrouping(), 0, 0));
            NVIC_EnableIRQ(USART1_IRQn);
            USART_InitStruct.BaudRate            = bound;
            USART_InitStruct.DataWidth           = wordLen;
            USART_InitStruct.StopBits            = stopBit;
            USART_InitStruct.Parity              = parity;
            USART_InitStruct.TransferDirection   = LL_USART_DIRECTION_TX_RX;
            USART_InitStruct.HardwareFlowControl = LL_USART_HWCONTROL_NONE;
            USART_InitStruct.OverSampling        = LL_USART_OVERSAMPLING_16;
            LL_USART_Init(USART1, &USART_InitStruct);

            LL_USART_SetTXFIFOThreshold(USART1, LL_USART_FIFOTHRESHOLD_8_8);
            LL_USART_SetRXFIFOThreshold(USART1, LL_USART_FIFOTHRESHOLD_8_8);
            LL_USART_EnableFIFO(USART1);

            LL_USART_ConfigAsyncMode(USART1);
            LL_USART_Enable(USART1);
            LL_USART_EnableIT_RXNE(USART1); // 接收中断
            LL_USART_EnableIT_PE(USART1);   // 奇偶校验错误中断

            // LL_USART_EnableIT_TXFE(USART1); //启用TXFIFO Empty中断
            LL_USART_DisableIT_TC(USART1);
            LL_USART_DisableIT_TXFE(USART1);

            COM1_Dev.inited = 1;
        } break;
        case COM2: {

            // 创建接收数据缓冲区
            QUEUE_PacketCreate(&m_QueueCom2Rx, m_Com2RxBuf, sizeof(m_Com2RxBuf));
            // 创建发送数据缓冲区
            QUEUE_PacketCreate(&m_QueueCom2Tx, m_Com2TxBuf, sizeof(m_Com2TxBuf));

            /* Peripheral clock enable */
            LL_APB1_GRP1_EnableClock(LL_APB1_GRP1_PERIPH_USART2);

            LL_AHB2_GRP1_EnableClock(LL_AHB2_GRP1_PERIPH_GPIOA);
            /**USART2 GPIO Configuration
            PA2   ------> USART2_TX
            PA3   ------> USART2_RX
            */
            GPIO_InitStruct.Pin        = LL_GPIO_PIN_2;
            GPIO_InitStruct.Mode       = LL_GPIO_MODE_ALTERNATE;
            GPIO_InitStruct.Speed      = LL_GPIO_SPEED_FREQ_LOW;
            GPIO_InitStruct.OutputType = LL_GPIO_OUTPUT_PUSHPULL;
            GPIO_InitStruct.Pull       = LL_GPIO_PULL_NO;
            GPIO_InitStruct.Alternate  = LL_GPIO_AF_7;
            LL_GPIO_Init(GPIOA, &GPIO_InitStruct);

            GPIO_InitStruct.Pin        = LL_GPIO_PIN_3;
            GPIO_InitStruct.Mode       = LL_GPIO_MODE_ALTERNATE;
            GPIO_InitStruct.Speed      = LL_GPIO_SPEED_FREQ_LOW;
            GPIO_InitStruct.OutputType = LL_GPIO_OUTPUT_PUSHPULL;
            GPIO_InitStruct.Pull       = LL_GPIO_PULL_NO;
            GPIO_InitStruct.Alternate  = LL_GPIO_AF_7;
            LL_GPIO_Init(GPIOA, &GPIO_InitStruct);

            /* USART2 interrupt Init */
            NVIC_SetPriority(USART2_IRQn, NVIC_EncodePriority(NVIC_GetPriorityGrouping(), 3, 0));
            NVIC_EnableIRQ(USART2_IRQn);

            USART_InitStruct.BaudRate            = bound;
            USART_InitStruct.DataWidth           = wordLen;
            USART_InitStruct.StopBits            = stopBit;
            USART_InitStruct.Parity              = parity;
            USART_InitStruct.TransferDirection   = LL_USART_DIRECTION_TX_RX;
            USART_InitStruct.HardwareFlowControl = LL_USART_HWCONTROL_NONE;
            USART_InitStruct.OverSampling        = LL_USART_OVERSAMPLING_16;
            LL_USART_Init(USART2, &USART_InitStruct);

            LL_USART_SetTXFIFOThreshold(USART2, LL_USART_FIFOTHRESHOLD_8_8);
            LL_USART_SetRXFIFOThreshold(USART2, LL_USART_FIFOTHRESHOLD_8_8);
            LL_USART_EnableFIFO(USART2);

            LL_USART_ConfigAsyncMode(USART2);
            LL_USART_Enable(USART2);
            LL_USART_EnableIT_RXNE(USART2); // 接收中断
            LL_USART_EnableIT_PE(USART2);   // 奇偶校验错误中断
            // LL_USART_EnableIT_TXFE(USART2); //启用TXFIFO Empty中断

            COM2_Dev.inited = 1;
        }
        case COM3: {
            // 创建接收数据缓冲区
            QUEUE_PacketCreate(&m_QueueCom3Rx, m_Com3RxBuf, sizeof(m_Com3RxBuf));
            // 创建发送数据缓冲区
            QUEUE_PacketCreate(&m_QueueCom3Tx, m_Com3TxBuf, sizeof(m_Com3TxBuf));
            /** Initializes the peripherals clocks
             */
            PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_USART3;
            PeriphClkInit.Usart3ClockSelection = RCC_USART3CLKSOURCE_PCLK1;
            if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit) != HAL_OK) {
                Error_Handler();
            }

            /* Peripheral clock enable */
            LL_APB1_GRP1_EnableClock(LL_APB1_GRP1_PERIPH_USART3);

            LL_AHB2_GRP1_EnableClock(LL_AHB2_GRP1_PERIPH_GPIOB);
            /**USART3 GPIO Configuration
            PB10   ------> USART3_TX
            PB11   ------> USART3_RX
            */
            GPIO_InitStruct.Pin        = LL_GPIO_PIN_10;
            GPIO_InitStruct.Mode       = LL_GPIO_MODE_ALTERNATE;
            GPIO_InitStruct.Speed      = LL_GPIO_SPEED_FREQ_LOW;
            GPIO_InitStruct.OutputType = LL_GPIO_OUTPUT_PUSHPULL;
            GPIO_InitStruct.Pull       = LL_GPIO_PULL_NO;
            GPIO_InitStruct.Alternate  = LL_GPIO_AF_7;
            LL_GPIO_Init(GPIOB, &GPIO_InitStruct);

            GPIO_InitStruct.Pin        = LL_GPIO_PIN_11;
            GPIO_InitStruct.Mode       = LL_GPIO_MODE_ALTERNATE;
            GPIO_InitStruct.Speed      = LL_GPIO_SPEED_FREQ_LOW;
            GPIO_InitStruct.OutputType = LL_GPIO_OUTPUT_PUSHPULL;
            GPIO_InitStruct.Pull       = LL_GPIO_PULL_NO;
            GPIO_InitStruct.Alternate  = LL_GPIO_AF_7;
            LL_GPIO_Init(GPIOB, &GPIO_InitStruct);

            /* USART3 interrupt Init */
            NVIC_SetPriority(USART3_IRQn, NVIC_EncodePriority(NVIC_GetPriorityGrouping(), 0, 0));
            NVIC_EnableIRQ(USART3_IRQn);

            USART_InitStruct.BaudRate            = bound;
            USART_InitStruct.DataWidth           = wordLen;
            USART_InitStruct.StopBits            = stopBit;
            USART_InitStruct.Parity              = parity;
            USART_InitStruct.TransferDirection   = LL_USART_DIRECTION_TX_RX;
            USART_InitStruct.HardwareFlowControl = LL_USART_HWCONTROL_NONE;
            USART_InitStruct.OverSampling        = LL_USART_OVERSAMPLING_16;
            LL_USART_Init(USART3, &USART_InitStruct);

            LL_USART_SetTXFIFOThreshold(USART3, LL_USART_FIFOTHRESHOLD_8_8);
            LL_USART_SetRXFIFOThreshold(USART3, LL_USART_FIFOTHRESHOLD_8_8);
            LL_USART_EnableFIFO(USART3);

            LL_USART_ConfigAsyncMode(USART3);
            LL_USART_Enable(USART3);
            LL_USART_EnableIT_RXNE(USART3); // 接收中断
            LL_USART_EnableIT_PE(USART3);   // 奇偶校验错误中断
            // LL_USART_EnableIT_TXFE(USART3); //启用TXFIFO Empty中断

            COM3_Dev.inited = 1;
        } break;

        default:
            break;
    }
}

/**
 * @brief 串口写操作
 *
 * @param comx 串口号
 * @param writeBuf 存放待写数据缓存区的指针
 * @param uLen 需要写多少个字节
 * @return uint32_t >0-写出去实际字节数，0-未初始化，写失败
 */
uint32_t Uart_Write(COMID_t comid, uint8_t *writeBuf, uint32_t uLen)
{
    uint32_t uRtn = 0;
    switch (comid) {
        case COM1: // 串口1
        {
            if (COM1_Dev.inited == 0)
                uRtn = 0;
            else
                uRtn = uLen;
            if (uRtn > 0) {
                uint8_t ch        = 0;
                uint32_t push_len = 0;
                for (int i = 0; i < uLen; i++) {
                    ch = writeBuf[i];
                    while (1) {
                        /* 将新数据填入发送缓冲区 */
                        DISABLE_INT();
                        push_len = QUEUE_PacketIn(&m_QueueCom1Tx, &ch, 1);
                        ENABLE_INT();
                        if (push_len > 0) {
                            break;
                        } else {
                            /* 数据已填满缓冲区 */
                            /* 如果发送缓冲区已经满了，则等待缓冲区空 */
                            LL_USART_EnableIT_TXE(USART1);
                        }
                    }
                }
                /* 使能发送中断（缓冲区空） */
                LL_USART_EnableIT_TXE(USART1);
            }
        } break;
        case COM2: // 串口2
        {
            if (COM2_Dev.inited == 0)
                uRtn = 0;
            else
                uRtn = uLen;
            if (uRtn > 0) {
                uint8_t ch        = 0;
                uint32_t push_len = 0;
                for (int i = 0; i < uLen; i++) {
                    ch = writeBuf[i];
                    while (1) {
                        /* 将新数据填入发送缓冲区 */
                        DISABLE_INT();
                        push_len = QUEUE_PacketIn(&m_QueueCom2Tx, &ch, 1);
                        ENABLE_INT();
                        if (push_len > 0) {
                            break;
                        } else {
                            /* 数据已填满缓冲区 */
                            /* 如果发送缓冲区已经满了，则等待缓冲区空 */
                            LL_USART_EnableIT_TXE(USART2);
                        }
                    }
                }
                /* 使能发送中断（缓冲区空） */
                LL_USART_EnableIT_TXE(USART2);
            }
        } break;
        case COM3: // 串口3
        {
            if (COM3_Dev.inited == 0)
                uRtn = 0;
            else
                uRtn = uLen;
            if (uRtn > 0) {
                uint8_t ch        = 0;
                uint32_t push_len = 0;
                for (int i = 0; i < uLen; i++) {
                    ch = writeBuf[i];
                    while (1) {
                        /* 将新数据填入发送缓冲区 */
                        DISABLE_INT();
                        push_len = QUEUE_PacketIn(&m_QueueCom3Tx, &ch, 1);
                        ENABLE_INT();
                        if (push_len > 0) {
                            break;
                        } else {
                            /* 数据已填满缓冲区 */
                            /* 如果发送缓冲区已经满了，则等待缓冲区空 */
                            LL_USART_EnableIT_TXE(USART3);
                        }
                    }
                }
                /* 使能发送中断（缓冲区空） */
                LL_USART_EnableIT_TXE(USART3);
            }
        } break;

        default:
            break;
    }
    return uRtn;
}

/**
 * @brief 串口读操作
 *
 * @param comx 串口号
 * @param pBuf 存放读取数据的缓存区的指针
 * @param uiLen 本次操作最多能读取的字节数
 * @return uint32_t >0-实际读取的字节数，0-没有数据或者串口不可用
 */
uint32_t Uart_Read(COMID_t comid, unsigned char *pBuf, uint32_t uiLen)
{

    int uRtn = 0;
    switch (comid) {
        case COM1: // 串口1
            uRtn = QUEUE_PacketOut(&m_QueueCom1Rx, pBuf, uiLen);
            break;
        case COM2: // 串口2
            uRtn = QUEUE_PacketOut(&m_QueueCom2Rx, pBuf, uiLen);
            break;
        case COM3: // 串口3
            uRtn = QUEUE_PacketOut(&m_QueueCom3Rx, pBuf, uiLen);
            break;
        case COM4: // 串口4
            uRtn = QUEUE_PacketOut(&m_QueueCom4Rx, pBuf, uiLen);
            break;
        case COM5: // 串口5
            uRtn = QUEUE_PacketOut(&m_QueueCom5Rx, pBuf, uiLen);
            break;
        case COM6: // 串口6
            uRtn = QUEUE_PacketOut(&m_QueueCom6Rx, pBuf, uiLen);
            break;
        default:
            break;
    }
    return uRtn;
}

/**
 * @brief 注册Uart_Write写入完成回调函数。需要注意的是两次Uart_Write时间间隔太短可能就不能
 * 对应每个Uart_Write调用产生中断。
 *
 * @param comid 串口号。
 * @param callback 回调函数指针。
 * @return uint8_t 成功1，失败0.
 */
uint8_t Uart_SetWriteOverCallback(COMID_t comid, UartWriteOverCallback_t callback)
{
    uint8_t ret = 0;
    switch (comid) {
        case COM1: // 串口1
            COM1_Dev.write_over_callback = callback;
            break;
        case COM2: // 串口2
            COM2_Dev.write_over_callback = callback;
            break;
        case COM3: // 串口3
            COM3_Dev.write_over_callback = callback;
            break;
        case COM4: // 串口4
            COM4_Dev.write_over_callback = callback;
            break;
        case COM5: // 串口5
            COM5_Dev.write_over_callback = callback;
            break;
        case COM6: // 串口6
            COM6_Dev.write_over_callback = callback;
            break;
        default:
            ret = 1;
            break;
    }
    return ret;
}

/**
 * @brief
 *
 * @param comid 串口号。
 * @param callback 回调函数指针。
 * @return uint8_t 成功1，失败0.
 */
uint8_t Uart_RegisterReceiveCharCallback(COMID_t comid, UartReceiveCharCallback_t callback)
{
    uint8_t ret = 0;
    switch (comid) {
        case COM1: // 串口1
            COM1_Dev.receive_char_callback = callback;
            break;
        case COM2: // 串口2
            COM2_Dev.receive_char_callback = callback;
            break;
        case COM3: // 串口3
            COM3_Dev.receive_char_callback = callback;
            break;
        case COM4: // 串口4
            COM4_Dev.receive_char_callback = callback;
            break;
        case COM5: // 串口5
            COM5_Dev.receive_char_callback = callback;
            break;
        case COM6: // 串口6
            COM6_Dev.receive_char_callback = callback;
            break;
        default:
            ret = 1;
            break;
    }
    return ret;
}

uint8_t Uart_RegisterReceiveCharNotReadCallback(COMID_t comid, void (*callback)(void))
{
    uint8_t ret = 0;
    switch (comid) {
        case COM1: // 串口1
            COM1_Dev.receive_char_callback_not_read = callback;
            break;
        case COM2: // 串口2
            COM2_Dev.receive_char_callback_not_read = callback;
            break;
        case COM3: // 串口3
            COM3_Dev.receive_char_callback_not_read = callback;
            break;
        case COM4: // 串口4
            COM4_Dev.receive_char_callback_not_read = callback;
            break;
        case COM5: // 串口5
            COM5_Dev.receive_char_callback_not_read = callback;
            break;
        case COM6: // 串口6
            COM6_Dev.receive_char_callback_not_read = callback;
            break;
        default:
            ret = 1;
            break;
    }
    return ret;
}

/**
 * @brief 取消注册中断中接收字符数据流的函数，这样就能且只能使用Uart_Read读取数据了。
 *
 * @param comid 串口号。
 * @return uint8_t 成功1，失败0.
 */
uint8_t Uart_UnregisterReceiveCharCallback(COMID_t comid)
{
    uint8_t ret = 0;
    switch (comid) {
        case COM1: // 串口1
            COM1_Dev.receive_char_callback = NULL;
            break;
        case COM2: // 串口2
            COM2_Dev.receive_char_callback = NULL;
            break;
        case COM3: // 串口3
            COM3_Dev.receive_char_callback = NULL;
            break;
        case COM4: // 串口4
            COM4_Dev.receive_char_callback = NULL;
            break;
        case COM5: // 串口5
            COM5_Dev.receive_char_callback = NULL;
            break;
        case COM6: // 串口6
            COM6_Dev.receive_char_callback = NULL;
            break;
        default:
            ret = 1;
            break;
    }
    return ret;
}

/**
 * @brief This function handles USART1 global interrupt.
 */
void USART1_IRQHandler(void)
{
    uint8_t buffer[1];
    if (LL_USART_IsActiveFlag_RXNE(USART1) != RESET) // 检测是否接收中断
    {
        // 如果外部单独注册了接收字符数据流的方法，那么就使用外部注册的方法
        if (COM1_Dev.receive_char_callback != NULL) {
            buffer[0] = LL_USART_ReceiveData8(USART1); // 读取出来接收到的数据
            COM1_Dev.receive_char_callback(buffer[0]);
        } else if (COM1_Dev.receive_char_callback_not_read != NULL) {
            COM1_Dev.receive_char_callback_not_read();
        } else {
            buffer[0] = LL_USART_ReceiveData8(USART1); // 读取出来接收到的数据
            QUEUE_PacketIn(&m_QueueCom1Rx, buffer, 1); // LOOP
        }
    }
    if (LL_USART_IsActiveFlag_ORE(USART1) != RESET) {
        buffer[0] = USART1->RDR;
        LL_USART_ClearFlag_ORE(USART1);
    }
    LL_USART_ClearFlag_FE(USART1); // Clear Framing Error Flag
    LL_USART_ClearFlag_PE(USART1); // 奇偶校验错误清除

    // LL_USART_IsEnabledIT_TC是为了防止接收字符时也进入发送中断
    uint8_t ch              = 0;
    uint32_t queque_pop_len = 0;
    /* 处理发送缓冲区空中断 */
    if (LL_USART_IsEnabledIT_TXE(USART1) && LL_USART_IsActiveFlag_TXE(USART1)) {
        queque_pop_len = QUEUE_PacketOut(&m_QueueCom1Tx, &ch, 1);
        if (queque_pop_len > 0) {
            LL_USART_TransmitData8(USART1, ch); // 把数据再从串口发送出去
        } else {
            /* 发送缓冲区的数据已取完时， 禁止发送缓冲区空中断 （注意：此时最后1个数据还未真正发送完毕）*/
            LL_USART_DisableIT_TXE(USART1);
            /* 使能数据发送完毕中断 */
            LL_USART_EnableIT_TC(USART1);
        }
    }

    /* 数据bit位全部发送完毕的中断 */
    if (LL_USART_IsEnabledIT_TC(USART1) && LL_USART_IsActiveFlag_TC(USART1)) {
        queque_pop_len = QUEUE_PacketOut(&m_QueueCom1Tx, &ch, 1);
        if (queque_pop_len == 0) {
            /* 如果发送FIFO的数据全部发送完毕，禁止数据发送完毕中断 */
            LL_USART_DisableIT_TC(USART1);
            if (COM1_Dev.write_over_callback != NULL) {
                COM1_Dev.write_over_callback();
            }
        } else {
            /* 正常情况下，不会进入此分支 */
            /* 如果发送FIFO的数据还未完毕，则从发送FIFO取1个数据写入发送数据寄存器 */
            LL_USART_TransmitData8(USART1, ch); // 把数据再从串口发送出去
        }
    }
}

/**
 * @brief This function handles USART2 global interrupt.
 */
void USART2_IRQHandler(void)
{
    uint8_t buffer[1];
    if (LL_USART_IsActiveFlag_RXNE(USART2) != RESET) // 检测是否接收中断
    {
        // 如果外部单独注册了接收字符数据流的方法，那么就使用外部注册的方法
        if (COM2_Dev.receive_char_callback != NULL) {
            buffer[0] = LL_USART_ReceiveData8(USART2); // 读取出来接收到的数据
            COM2_Dev.receive_char_callback(buffer[0]);
        } else if (COM2_Dev.receive_char_callback_not_read != NULL) {
            COM2_Dev.receive_char_callback_not_read();
        } else {
            buffer[0] = LL_USART_ReceiveData8(USART2); // 读取出来接收到的数据
            QUEUE_PacketIn(&m_QueueCom1Rx, buffer, 1); // LOOP
        }
    }
    if (LL_USART_IsActiveFlag_ORE(USART2) != RESET) {
        buffer[0] = USART2->RDR;
        LL_USART_ClearFlag_ORE(USART2);
    }
    LL_USART_ClearFlag_FE(USART2); // Clear Framing Error Flag
    LL_USART_ClearFlag_PE(USART2); // 奇偶校验错误清除

    // LL_USART_IsEnabledIT_TC是为了防止接收字符时也进入发送中断
    uint8_t ch              = 0;
    uint32_t queque_pop_len = 0;
    /* 处理发送缓冲区空中断 */
    if (LL_USART_IsEnabledIT_TXE(USART2) && LL_USART_IsActiveFlag_TXE(USART2)) {
        queque_pop_len = QUEUE_PacketOut(&m_QueueCom2Tx, &ch, 1);
        if (queque_pop_len > 0) {
            LL_USART_TransmitData8(USART2, ch); // 把数据再从串口发送出去
        } else {
            /* 发送缓冲区的数据已取完时， 禁止发送缓冲区空中断 （注意：此时最后1个数据还未真正发送完毕）*/
            LL_USART_DisableIT_TXE(USART2);
            /* 使能数据发送完毕中断 */
            LL_USART_EnableIT_TC(USART2);
        }
    }

    /* 数据bit位全部发送完毕的中断 */
    if (LL_USART_IsEnabledIT_TC(USART2) && LL_USART_IsActiveFlag_TC(USART2)) {
        queque_pop_len = QUEUE_PacketOut(&m_QueueCom2Tx, &ch, 1);
        if (queque_pop_len == 0) {
            /* 如果发送FIFO的数据全部发送完毕，禁止数据发送完毕中断 */
            LL_USART_DisableIT_TC(USART2);
            if (COM2_Dev.write_over_callback != NULL) {
                COM2_Dev.write_over_callback();
            }
        } else {
            /* 正常情况下，不会进入此分支 */
            /* 如果发送FIFO的数据还未完毕，则从发送FIFO取1个数据写入发送数据寄存器 */
            LL_USART_TransmitData8(USART2, ch); // 把数据再从串口发送出去
        }
    }
}

/**
 * @brief This function handles USART3 global interrupt.
 */
void USART3_IRQHandler(void)
{
    uint8_t buffer[1];
    if (LL_USART_IsActiveFlag_RXNE(USART3) != RESET) // 检测是否接收中断
    {
        // 如果外部单独注册了接收字符数据流的方法，那么就使用外部注册的方法
        if (COM3_Dev.receive_char_callback != NULL) {
            buffer[0] = LL_USART_ReceiveData8(USART3); // 读取出来接收到的数据
            COM3_Dev.receive_char_callback(buffer[0]);
        } else if (COM3_Dev.receive_char_callback_not_read != NULL) {
            COM3_Dev.receive_char_callback_not_read();
        } else {
            buffer[0] = LL_USART_ReceiveData8(USART3); // 读取出来接收到的数据
            QUEUE_PacketIn(&m_QueueCom3Rx, buffer, 1); // LOOP
        }
    }
    if (LL_USART_IsActiveFlag_ORE(USART3) != RESET) {
        buffer[0] = USART3->RDR;
        LL_USART_ClearFlag_ORE(USART3);
    }
    LL_USART_ClearFlag_FE(USART3); // Clear Framing Error Flag
    LL_USART_ClearFlag_PE(USART3); // 奇偶校验错误清除

    if (LL_USART_IsEnabledIT_TC(USART3) && LL_USART_IsActiveFlag_TC(USART3)) {
        if (COM3_Dev.write_over_callback != NULL) {
            COM3_Dev.write_over_callback();
        }
    }
    
    // // LL_USART_IsEnabledIT_TC是为了防止接收字符时也进入发送中断
    // uint8_t ch              = 0;
    // uint32_t queque_pop_len = 0;
    // /* 处理发送缓冲区空中断 */
    // if (LL_USART_IsEnabledIT_TXE(USART3) && LL_USART_IsActiveFlag_TXE(USART3)) {
    //     queque_pop_len = QUEUE_PacketOut(&m_QueueCom3Tx, &ch, 1);
    //     if (queque_pop_len > 0) {
    //         LL_USART_TransmitData8(USART3, ch); // 把数据再从串口发送出去
    //     } else {
    //         /* 发送缓冲区的数据已取完时， 禁止发送缓冲区空中断 （注意：此时最后1个数据还未真正发送完毕）*/
    //         LL_USART_DisableIT_TXE(USART3);
    //         /* 使能数据发送完毕中断 */
    //         LL_USART_EnableIT_TC(USART3);
    //     }
    // }

    // /* 数据bit位全部发送完毕的中断 */
    // if (LL_USART_IsEnabledIT_TC(USART3) && LL_USART_IsActiveFlag_TC(USART3)) {
    //     queque_pop_len = QUEUE_PacketOut(&m_QueueCom1Tx, &ch, 1);
    //     if (queque_pop_len == 0) {
    //         /* 如果发送FIFO的数据全部发送完毕，禁止数据发送完毕中断 */
    //         LL_USART_DisableIT_TC(USART3);
    //         if (COM3_Dev.write_over_callback != NULL) {
    //             COM3_Dev.write_over_callback();
    //         }
    //     } else {
    //         /* 正常情况下，不会进入此分支 */
    //         /* 如果发送FIFO的数据还未完毕，则从发送FIFO取1个数据写入发送数据寄存器 */
    //         LL_USART_TransmitData8(USART3, ch); // 把数据再从串口发送出去
    //     }
    // }
}
