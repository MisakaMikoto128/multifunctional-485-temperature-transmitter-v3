/**
* @file CHIP_MCP4017T_104.h
* @author Liu Yuanlin (liuyuanlins@outlook.com)
* @brief
* @version 0.1
* @date 2024-03-15
* @last modified 2024-03-15
*
* @copyright Copyright (c) 2024 Liu Yuanlin Personal.
*
*/
#ifndef CHIP_MCP4017T_104_H
#define CHIP_MCP4017T_104_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stdbool.h>
#include "i2c.h"

#define RW_MCP4017T_104  100 //100Ω
#define RAB_MCP4017T_104  100000 //100KΩ
#define RS_MCP4017T_104  (RAB_MCP4017T_104/127)

#define MCP4017T_104_IIC_ADDR (0x2F) //7bit address

#define MCP4017T_104_WIPER_MAX 127
#define MCP4017T_104_WIPER_MIN 0

struct MCP4017T_104_Chip_t
{
    uint8_t wiper_setting;
    I2C_HandleTypeDef *pI2c;
};

/**
 * @brief 
 * 
 * @return uint32_t 0:success, other:fail
 */
int32_t CHIP_MCP4017T_104_Init(struct MCP4017T_104_Chip_t *pChip);

/**
 * @brief 设置Wiper位置，范围0~127
 * 
 * @param pChip 
 * @param wiper_setting 0-127
 * @return int32_t 0:success, other:fail
 */
int32_t CHIP_MCP4017T_104_SetWiper(struct MCP4017T_104_Chip_t *pChip, uint8_t wiper_setting);


/**
 * @brief 读取Wiper当前的位置，范围0~127
 * 
 * @param pChip 
 * @param wiper_setting 
 * @return int32_t 0:success, other:fail
 */
int32_t CHIP_MCP4017T_104_GetWiper(struct MCP4017T_104_Chip_t *pChip, uint8_t *wiper_setting);

#ifdef __cplusplus
}
#endif
#endif //!CHIP_MCP4017T_104_H
