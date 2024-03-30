/**
* @file CHIP_TMP102.h
* @author Liu Yuanlin (liuyuanlins@outlook.com)
* @brief
* @version 0.1
* @date 2024-03-29
* @last modified 2024-03-29
*
* @copyright Copyright (c) 2024 Liu Yuanlin Personal.
*
*/
#ifndef CHIP_TMP102_H
#define CHIP_TMP102_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stdbool.h>

bool CHIP_TMP102_Init();
/**
 * @brief 
 * 
 * @return true 
 * @return false 
 */
bool CHIP_TMP102_GetTemperature(float *pTemperature);

#ifdef __cplusplus
}
#endif
#endif //!CHIP_TMP102_H
