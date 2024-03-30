/**
 * @file CHIP_TMP102.c
 * @author Liu Yuanlin (liuyuanlins@outlook.com)
 * @brief
 * @version 0.1
 * @date 2024-03-29
 * @last modified 2024-03-29
 *
 * @copyright Copyright (c) 2024 Liu Yuanlin Personal.
 *
 */
#include "CHIP_TMP102.h"
#include "i2c.h"
#include "log.h"

#define TMP102_I2C_HANDLE hi2c1
#define TMP102_SLAVE_ADDR 0x48 // 01001000(最高位无用、ADD0接GND)

// TMP102 register addresses
#define TMP102_TEMPERATURE_REG_ADDR 0x00
#define TMP102_CONFI_REG_ADDR       0x01

#define TMP102_CONFI_REG_BYTE1(OS, R1, R0, F1, F0, POL, TM, SD) ((OS << 7) | (R1 << 6) | (R0 << 5) | (F1 << 4) | (F0 << 3) | (POL << 2) | (TM << 1) | SD
#define TMP102_CONFI_REG_BYTE2(AL, CR, EM) ((AL << 5) | (CR << 4) | (EM << 3))

// Convert to 2's complement, since temperature can be negative
#define TMP102_CHECKSIGN_12BIT(value) \
    if (value > 0x7FF) { value |= 0xF000; }
#define TMP102_CHECKSIGN_13BIT(value) \
    if (value > 0xFFF) { value |= 0xE000; }

bool CHIP_TMP102_Init()
{
    /*
    默认配置为12位精度，连续转换模式，
    此模式未经校准时的精度：
        ––25°C 至 85°C 范围内为 2.0°C（最大值）

        Byte1的各位数含义：
        OS：“0”表示从转换模式进入关断模式；“1”表示从关断模式进行一次温度数据转换。
        R1/R0：转换器分辨率位，默认0x11，表示12位分辨率。
        F1/F0：温度超出High/Low寄存器限制时而存在。见Table12。
        POL：Alert Pin的工作方式，与Byte2的AL搭配。见Figure 13。
        TM：“0”表示比较模式；“1”表示中断模式。
        SD：关断模式（Shutdown mode），“0”表示维持数据转换（ADC）状态，“1”表示现有数据转换完成，关断转换状态。

        Byte2的各位数含义：
        CR1/CR0：数据转换（ADC）速率。见Table 5。
        AL：与Byte1的POL搭配。见figure13。
        EM：“0”表示正常工作模式，数据格式12位；“1”表示扩展工作模式，数据格式13位。
    */

    uint8_t config[2]     = {0x60, 0xA0}; // 0x60: 12位精度，连续转换模式，0xA0: 12位精度，连续转换模式，警报输出无效
    HAL_StatusTypeDef ret = HAL_OK;
    // 写入配置寄存器
    ret = HAL_I2C_Mem_Write(&TMP102_I2C_HANDLE, TMP102_SLAVE_ADDR << 1, TMP102_CONFI_REG_ADDR, I2C_MEMADD_SIZE_8BIT, (uint8_t *)&config, 2, 1000);
    if (ret != HAL_OK) {
        ULOG_ERROR("TMP102 write config register failed");
        return false;
    }

    return true;
}

bool CHIP_TMP102_GetTemperature(float *pTemperature)
{
    int16_t temp = 0;

    // 12bit 温度数据
    uint8_t value[2]      = {0};
    HAL_StatusTypeDef ret = HAL_OK;
    ret                   = HAL_I2C_Mem_Read(&TMP102_I2C_HANDLE, TMP102_SLAVE_ADDR << 1, TMP102_TEMPERATURE_REG_ADDR, I2C_MEMADD_SIZE_8BIT, value, 2, 10);
    if (ret != HAL_OK) {
        ULOG_ERROR("TMP102 read temperature register failed");
        return false;
    }

    temp = (value[0] << 4) | (value[1] >> 4);
    // check if value is negative
    TMP102_CHECKSIGN_12BIT(temp);
    float temperature = temp * 0.0625f;
    *pTemperature     = temperature;
    return true;
}