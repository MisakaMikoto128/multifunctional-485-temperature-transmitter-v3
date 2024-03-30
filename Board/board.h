/**
* @file board.h
* @author Liu Yuanlin (liuyuanlins@outlook.com)
* @brief
* @version 0.1
* @date 2024-03-14
* @last modified 2024-03-14
*
* @copyright Copyright (c) 2024 Liu Yuanlin Personal.
*
*/
#ifndef BOARD_H
#define BOARD_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stdbool.h>

#include "gpio.h"


/**
 * @brief 主板初始化。
 * 
 */
void Board_Init();
#ifdef __cplusplus
}
#endif
#endif //!BOARD_H
