/**
* @file CHIP_TTL_Screen.h
* @author Liu Yuanlin (liuyuanlins@outlook.com)
* @brief
* @version 0.1
* @date 2024-03-28
* @last modified 2024-03-28
*
* @copyright Copyright (c) 2024 Liu Yuanlin Personal.
*
*/
#ifndef CHIP_TTL_SCREEN_H
#define CHIP_TTL_SCREEN_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stdbool.h>

void CHIP_TTL_Screen_Init();
/*
串口协议说明：
波特率9600、8位数据位、1位停止位、无校验，ASCII码格式
比如上位机发送：X1234105
设置成功返回：OK
解释：
第一个字符大写字符“X”固定的
第二个字符数码管显示的千位
第三个字符数码管显示的百位
第四个字符数码管显示的十位
第五个字符数码管显示的个位
第六个字符数码管显示小数点位置
第七个字符收到数据是否掉电保存，0不保存，1保存
第八个字符数码管亮度设置，八级亮度可调1到8，数字越大越亮
总结：表示显示的是1.234，数据断电后不保存，亮度等级5
*/

/**
 * @brief
 *
 * @param value
 * @return true
 * @return false
 */
bool TTL_ScreenPrint(float value);

void CHIP_TTL_Screen_Test();
#ifdef __cplusplus
}
#endif
#endif //!CHIP_TTL_SCREEN_H
