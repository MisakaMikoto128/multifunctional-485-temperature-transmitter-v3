/**
* @file CHIP_TCA9548A.h
* @author Liu Yuanlin (liuyuanlins@outlook.com)
* @brief
* @version 0.1
* @date 2024-03-29
* @last modified 2024-03-29
*
* @copyright Copyright (c) 2024 Liu Yuanlin Personal.
*
*/
#ifndef CHIP_TCA9548A_H
#define CHIP_TCA9548A_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stdbool.h>

/*********************************************************************
 * DEFINITIONS
 */
#define TCA9548A_SLAVE_ADDR         0x70

#define TCA9548A_WRITE_BIT          0x00
#define TCA9548A_READ_BIT           0x01

#define TCA9548A_CHANNEL_0          0x01
#define TCA9548A_CHANNEL_1          0x02
#define TCA9548A_CHANNEL_2          0x04
#define TCA9548A_CHANNEL_3          0x08
#define TCA9548A_CHANNEL_4          0x10
#define TCA9548A_CHANNEL_5          0x20
#define TCA9548A_CHANNEL_6          0x40
#define TCA9548A_CHANNEL_7          0x80


void CHIP_TCA9548A_Init();
void CHIP_TCA9548A_SelectChannel(uint8_t channel);

#ifdef __cplusplus
}
#endif
#endif //!CHIP_TCA9548A_H
