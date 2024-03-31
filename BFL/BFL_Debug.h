/**
* @file BFL_Debug.h
* @author Liu Yuanlin (liuyuanlins@outlook.com)
* @brief
* @version 0.1
* @date 2024-03-30
* @last modified 2024-03-30
*
* @copyright Copyright (c) 2024 Liu Yuanlin Personal.
*
*/
#ifndef BFL_DEBUG_H
#define BFL_DEBUG_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stdbool.h>

void BFL_Debug_Init();
void BFL_Debug_SignalPinSet();
void BFL_Debug_SignalPinReset();
void BFL_Debug_SignalPinToggle();
#ifdef __cplusplus
}
#endif
#endif //!BFL_DEBUG_H
