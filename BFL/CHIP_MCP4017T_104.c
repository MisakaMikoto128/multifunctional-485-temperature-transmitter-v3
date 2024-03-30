/**
 * @file CHIP_MCP4017T_104.c
 * @author Liu Yuanlin (liuyuanlins@outlook.com)
 * @brief
 * @version 0.1
 * @date 2024-03-15
 * @last modified 2024-03-15
 *
 * @copyright Copyright (c) 2024 Liu Yuanlin Personal.
 *
 */
#include "CHIP_MCP4017T_104.h"
#include "i2c.h"
#include "HDL_G4_CPU_Time.h"

#define MCP4017T_104_I2C_TIMEOUT 100 // ms
#define MCP4017T_104_DEFAULT_WIPER MCP4017T_104_WIPER_MAX
#define MCP4017T_104_USE_HD_I2C  1

/**
 * @brief
 *
 * @param pChip
 * @param wiper_setting
 * @return int32_t 0:success, other:fail
 */
static int32_t MCP4017T_104_I2C_Write(struct MCP4017T_104_Chip_t *pChip, uint8_t wiper_setting)
{
#if MCP4017T_104_USE_HD_I2C == 1
    HAL_StatusTypeDef status = HAL_I2C_Master_Transmit(pChip->pI2c, MCP4017T_104_IIC_ADDR << 1, &wiper_setting, 1, MCP4017T_104_I2C_TIMEOUT);

    if (status != HAL_OK) {
        return -1;
    }
    pChip->wiper_setting = wiper_setting;
    return 0;
#else
    IIC_Start();
    IIC_Send_Byte(MCP4017T_104_IIC_ADDR << 1 | 0x00);
    if (IIC_Wait_Ack()) {
        IIC_Stop();
        return -1;
    }
    IIC_Send_Byte(wiper_setting);
    if (IIC_Wait_Ack()) {
        IIC_Stop();
        return -1;
    }
    IIC_Stop();
    pChip->wiper_setting = wiper_setting;
    return 0;
#endif
}

/**
 * @brief
 *
 * @param pChip
 * @param wiper_setting
 * @return int32_t 0:success, other:fail
 */
static int32_t MCP4017T_104_I2C_Read(struct MCP4017T_104_Chip_t *pChip, uint8_t *wiper_setting)
{
#if MCP4017T_104_USE_HD_I2C == 1
    uint8_t data[1];
    HAL_StatusTypeDef status = HAL_I2C_Master_Receive(pChip->pI2c, MCP4017T_104_IIC_ADDR << 1, data, 1, MCP4017T_104_I2C_TIMEOUT);

    if (status != HAL_OK) {
        return -1;
    }
    *wiper_setting       = data[0];
    pChip->wiper_setting = data[0];
    return 0;
#else
    IIC_Start();
    IIC_Send_Byte(MCP4017T_104_IIC_ADDR << 1 | 0x01);
    if (IIC_Wait_Ack()) {
        IIC_Stop();
        return -1;
    }
    *wiper_setting       = IIC_Read_Byte(0);
    pChip->wiper_setting = *wiper_setting;
    IIC_Stop();
    return 0;
#endif
}

/**
 * @brief
 *
 * @return uint32_t 0:success, other:fail
 */
int32_t CHIP_MCP4017T_104_Init(struct MCP4017T_104_Chip_t *pChip)
{
#if MCP4017T_104_USE_HD_I2C == 1
    
#else
		IIC_Init();
#endif
    // 确保I2C总线已经初始化了
    pChip->wiper_setting = MCP4017T_104_DEFAULT_WIPER;
    return MCP4017T_104_I2C_Write(pChip, pChip->wiper_setting);
}

/**
 * @brief 设置Wiper位置，范围0~127
 *
 * @param pChip
 * @param wiper_setting 0-127
 * @return int32_t
 */
int32_t CHIP_MCP4017T_104_SetWiper(struct MCP4017T_104_Chip_t *pChip, uint8_t wiper_setting)
{
    if (wiper_setting > MCP4017T_104_WIPER_MAX) {
        wiper_setting = MCP4017T_104_WIPER_MAX;
    }
    if (wiper_setting < MCP4017T_104_WIPER_MIN) {
        wiper_setting = MCP4017T_104_WIPER_MIN;
    }
    return MCP4017T_104_I2C_Write(pChip, wiper_setting);
}

int32_t CHIP_MCP4017T_104_GetWiper(struct MCP4017T_104_Chip_t *pChip, uint8_t *wiper_setting)
{
    return MCP4017T_104_I2C_Read(pChip, wiper_setting);
}
