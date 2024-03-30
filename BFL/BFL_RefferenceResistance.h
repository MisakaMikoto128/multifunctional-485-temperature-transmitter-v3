/**
* @file BFL_RefferenceResistance.h
* @author Liu Yuanlin (liuyuanlins@outlook.com)
* @brief
* @version 0.1
* @date 2024-03-30
* @last modified 2024-03-30
*
* @copyright Copyright (c) 2024 Liu Yuanlin Personal.
*
*/
#ifndef BFL_REFFERENCERESISTANCE_H
#define BFL_REFFERENCERESISTANCE_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stdbool.h>


#define RREF1_RESISTANCE_50K 1
#define RREF1_RESISTANCE_10K 2
#define RREF1_RESISTANCE_2K 3
#define RREF1_RESISTANCE_NONE 0

void RefferenceResistanceSelect(int Resistance_ID);
void RefferenceResistanceInit();
/**
 * @brief 获取当前电阻ID
 * 
 * @return int 
 */
int RefferenceResistanceGetCurrent();
/**
 * @brief 
 * 
 * @return int -1代表无穷大，其他值代表电阻值
 */
int RefferenceResistanceValueGetCurrent();

#ifdef __cplusplus
}
#endif
#endif //!BFL_REFFERENCERESISTANCE_H
