/**
* @file ntc_resistance.c
* @author Liu Yuanlin (liuyuanlins@outlook.com)
* @brief
* @version 0.1
* @date 2024-03-28
* @last modified 2024-03-28
*
* @copyright Copyright (c) 2024 Liu Yuanlin Personal.
*
*/
#include "ntc_resistance.h"
#include <math.h>

/**
 * @brief
 *
 * @param _R1       当前温度下的电阻
 * @param _B        所使用的NTC电阻B值(datasheet里面有,例如3950)
 * @param _R2       T2温度下的电阻
 * @param _T2       一般是25℃
 * @return float    返回的就是当前温度(℃)
 */
float resistanceToTemperature(float _R1, float _B, float _R2, float _T2)
{
    return (1.0f / ((1.0f / _B) * log(_R1 / _R2) + (1.0f / (_T2 + 273.15f))) - 273.15f);
}


/**
 * @brief NTC B3950 50K/25℃
 *
 * @param _R1       当前温度下的电阻,kΩ
 * @param _B        所使用的NTC电阻B值(datasheet里面有,例如3950)
 * @param _R2       T2温度下的电阻
 * @param _T2       一般是25℃
 * @return float    返回的就是当前温度(℃)
 */
float resistanceToTemperature_B3950(float _R1)
{
    // return (1.0f / ((1.0f / 4091.10985f) * log(_R1 / 50) + (1.0f / (25.0f + 273.15f))) - 273.15f);
    return (1.0f / ((1.0f / 4091.10985f) * log(_R1 / 50) + (1.0f / (25.0f + 273.15f))) - 273.15f);
}
