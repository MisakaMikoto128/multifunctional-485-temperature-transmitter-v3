/**
* @file CHIP_TTL_Screen.c
* @author Liu Yuanlin (liuyuanlins@outlook.com)
* @brief
* @version 0.1
* @date 2024-03-28
* @last modified 2024-03-28
*
* @copyright Copyright (c) 2024 Liu Yuanlin Personal.
*
*/
#include "CHIP_TTL_Screen.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h> //需要使用标准数学库中函数
#include "HDL_G4_Uart.h"
#include "HDL_G4_CPU_Time.h"

#define CHIP_TTL_SCREEN_COM COM2
/**
 * @brief
 *
 * @param value
 * @return true
 * @return false
 */
bool CHIP_TTL_ScreenPrint(float value)
{
#define TTL_SCREEN_MAX_COMMAND_LEN 11
    char command[TTL_SCREEN_MAX_COMMAND_LEN];
    int decimalPointPos;
    int saveOnPowerOff  = 1; // 假设数据断电后保存
    int brightnessLevel = 8; // 假设亮度等级为最高

    // 根据数值的大小和符号确定小数点位置和显示格式
    if (value < 0) {
        if (value <= -10.0f) {
            // 负数，两位整数，一位小数
            decimalPointPos = 3; // 小数点在第三位
            snprintf(command, TTL_SCREEN_MAX_COMMAND_LEN, "X%4.1f%1d%1d%1d", value, decimalPointPos, saveOnPowerOff, brightnessLevel);
        } else {
            // 负数，一位整数，两位小数
            decimalPointPos = 2; // 小数点在第二位
            snprintf(command, TTL_SCREEN_MAX_COMMAND_LEN, "X%4.2f%1d%1d%1d", value, decimalPointPos, saveOnPowerOff, brightnessLevel);
        }
    } else {
        if (value >= 100.0f) {
            // 正数，三位整数，一位小数
            decimalPointPos = 3; // 小数点在第三位
            snprintf(command, TTL_SCREEN_MAX_COMMAND_LEN, "X%4.1f%1d%1d%1d", value, decimalPointPos, saveOnPowerOff, brightnessLevel);
        } else if (value >= 10.0f) {
            // 正数，两位整数，二位小数
            decimalPointPos = 2; // 小数点在第三位
            snprintf(command, TTL_SCREEN_MAX_COMMAND_LEN, "X%5.2f%1d%1d%1d", value, decimalPointPos, saveOnPowerOff, brightnessLevel);
        } else {
            // 正数，一位整数，三位小数
            decimalPointPos = 1; // 小数点在第二位
            snprintf(command, TTL_SCREEN_MAX_COMMAND_LEN, "X%5.3f%1d%1d%1d", value, decimalPointPos, saveOnPowerOff, brightnessLevel);
        }
    }

    // 移除字符串中的小数点
    for (int i = 0; command[i] != '\0'; i++) {
        if (command[i] == '.') {
            for (int j = i; command[j] != '\0'; j++) {
                command[j] = command[j + 1];
            }
            break;
        }
    }

    for (int i = 0; command[i] != '\0'; i++) {
        // 替换'-'为'F'
        if (command[i] == '-') {
            command[i] = 'F';
        }
        // 替换' '为'0'
        if (command[i] == ' ') {
            command[i] = '\0';
            break;
        }
    }

    // 发送命令到数码管（这里假设你已经设置好了串口）
    // 例如：serial_send(command);
    Uart_Write(CHIP_TTL_SCREEN_COM, (uint8_t *)command, strlen(command));
    return true; // 假设发送总是成功
}

void CHIP_TTL_Screen_Test()
{
    float value = 12.34;
    while (1) {
        CHIP_TTL_ScreenPrint(value);
        value += 0.01;
        HDL_G4_CPU_Time_DelayMs(1000);
    }
}