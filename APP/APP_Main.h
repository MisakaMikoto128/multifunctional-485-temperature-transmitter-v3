/**
* @file APP_Main.h
* @author Liu Yuanlin (liuyuanlins@outlook.com)
* @brief
* @version 0.1
* @date 2024-03-28
* @last modified 2024-03-28
*
* @copyright Copyright (c) 2024 Liu Yuanlin Personal.
*
*/
#ifndef APP_MAIN_H
#define APP_MAIN_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stdbool.h>

#include "asyn_sys.h"

void APP_Main_Init();

typedef struct tagSysInfo_t {
    float Temp1;
    float Temp2;
    float Temp3;
    float Temp4;
    float Temp5;

    float TempD1;
    float TempD2;
    float TempD3;
    float TempD4;
    float TempD5;

    bool isT1Connected;
    bool isT2Connected;
    bool isT3Connected;
    bool isT4Connected;
    bool isT5Connected;

    bool isD1Connected;
    bool isD2Connected;
    bool isD3Connected;
    bool isD4Connected;
    bool isD5Connected;
} SysInfo_t;


void SysInfoWriteToRegHoldingBuf(const SysInfo_t *pSysInfo);
#ifdef __cplusplus
}
#endif
#endif //!APP_MAIN_H
