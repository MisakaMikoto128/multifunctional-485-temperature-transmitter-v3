/**
 * @file porttcp.c
 * @author Liu Yuanlin (liuyuanlins@outlook.com)
 * @brief
 * @version 0.1
 * @date 2023-12-22
 * @last modified 2023-12-22
 *
 * @copyright Copyright (c) 2023 Liu Yuanlin Personal.
 *
 */

/* ----------------------- System includes ----------------------------------*/
#include <stdio.h>
#include <string.h>
#include "port.h"
#include "w5500.h"
#include "socket.h"
#include "log.h"

/* ----------------------- Modbus includes ----------------------------------*/
#include "mb.h"
#include "mbport.h"

/* ----------------------- MBAP Header --------------------------------------*/

#define MB_TCP_DEFAULT_SOCKET 1

#define MB_TCP_DEFAULT_PORT   502
#define MB_TCP_BUF_SIZE       (256 + 7)

static uint8_t g_tcp_socket = MB_TCP_DEFAULT_SOCKET;
static uint16_t g_tcp_port   = MB_TCP_DEFAULT_PORT;

/* ----------------------- Prototypes ---------------------------------------*/

uint8_t ucTCPRequestFrame[MB_TCP_BUF_SIZE]; // Регистр приема
uint16_t ucTCPRequestLen;
uint8_t ucTCPResponseFrame[MB_TCP_BUF_SIZE]; // Отправляем регистр
uint16_t ucTCPResponseLen;
uint8_t bFrameSent = FALSE;

BOOL xMBTCPPortInit(USHORT usTCPPort)
{
    BOOL bOkay = FALSE;

    USHORT usPort;
    if (usTCPPort == 0) {
        usPort = g_tcp_port;
    } else {
        usPort     = (USHORT)usTCPPort;
        g_tcp_port = usTCPPort;
    }
    ULOG_INFO("Creating socket...\r\n");
    uint8_t mb_tcp_socket = g_tcp_socket;
    uint8_t code          = socket(mb_tcp_socket, Sn_MR_TCP, usPort, 0);
    if (code != mb_tcp_socket) {
        ULOG_INFO("socket() failed, code = %d\r\n", code);
        return bOkay;
    }
    listen(mb_tcp_socket);
    bOkay = TRUE;
    return bOkay;
}

void vMBTCPPortClose()
{
    uint8_t mb_tcp_socket = g_tcp_socket;
    close(mb_tcp_socket);
}

void vMBTCPPortDisable(void)
{
    uint8_t mb_tcp_socket = g_tcp_socket;
    close(mb_tcp_socket);
}

BOOL xMBTCPPortGetRequest(UCHAR **ppucMBTCPFrame, USHORT *usTCPLength)
{
    *ppucMBTCPFrame = (uint8_t *)&ucTCPRequestFrame[0];
    *usTCPLength    = ucTCPRequestLen;
    /* Reset the buffer. */
    ucTCPRequestLen = 0;
    return TRUE;
}

BOOL xMBTCPPortSendResponse(const UCHAR *pucMBTCPFrame, USHORT usTCPLength)
{
    memcpy(ucTCPResponseFrame, pucMBTCPFrame, usTCPLength);
    ucTCPResponseLen = usTCPLength;
    bFrameSent       = TRUE;
    return bFrameSent;
    return TRUE;
}

void modbus_tcps()
{
    uint8_t sn    = g_tcp_socket;
    uint16_t port = g_tcp_port;
    switch (getSn_SR(sn)) {
        case SOCK_CLOSED:
            socket(sn, Sn_MR_TCP, port, 0x00);
            break;
        case SOCK_INIT:
            listen(sn);
        case SOCK_ESTABLISHED:
            if (getSn_IR(sn) & Sn_IR_CON) {
                setSn_IR(sn, Sn_IR_CON);
            }
            ucTCPRequestLen = getSn_RX_RSR(sn);
            if (ucTCPRequestLen > 0) {
                recv(sn, ucTCPRequestFrame, ucTCPRequestLen);
                xMBPortEventPost(EV_FRAME_RECEIVED);
                if (bFrameSent) {
                    bFrameSent = FALSE;
                    send(sn, ucTCPResponseFrame, ucTCPResponseLen);
                }
            }
            break;
        case SOCK_CLOSE_WAIT:
            disconnect(sn);
            break;
        default:
            break;
    }
}