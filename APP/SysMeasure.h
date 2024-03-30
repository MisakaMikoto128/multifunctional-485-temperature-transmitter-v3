/**
 * @file SysMeasure.h
 * @author Liu Yuanlin (liuyuanlins@outlook.com)
 * @brief
 * @version 0.1
 * @date 2024-01-23
 * @last modified 2024-01-23
 *
 * @copyright Copyright (c) 2024 Liu Yuanlin Personal.
 *
 */
#ifndef SYSMEASURE_H
#define SYSMEASURE_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stdbool.h>
#include "average_filter.h"

#define ADC_CHANNEL_NUM         1
#define ADC_SAMPLE_NUM          (4096 * 2)
#define VREF_VOLT               3.291f
#define VREF_FACTOR             0.9727f

#define FFT_POINT_NUM           (ADC_CHANNEL_NUM * ADC_SAMPLE_NUM)

#define Fs                      200000                        // Hz
#define FrqSamp                 50                            // 被采样波形的频率
#define DC_IDEX                 0                             // 变换结果中直流分量的位置
#define IDEX_50                 (50 * FFT_POINT_NUM / Fs)     // 变换结果中50Hz分量的位置
#define CIDEX_50                (50 * 2 * FFT_POINT_NUM / Fs) // 变换结果中50Hz分量的位置

#define ADC_SAMPLE_ONE_GRUP_NUM 400

#define MCP4017T_104_NUM 6
extern struct MCP4017T_104_Chip_t g_mcp4017t_104_chip_list[MCP4017T_104_NUM];
extern uint8_t g_mcp4017t_i2c_channel_map[MCP4017T_104_NUM];
struct SysMeasureData_t {
    float sensor1;
    average_filter_t sensor1Filter;
    uint16_t adc1_sample_buf[ADC_SAMPLE_ONE_GRUP_NUM];
    uint32_t sensor1SwitchCurrentMoment; // Unit: ms
    float sensor2;
    average_filter_t sensor2Filter;
    uint16_t adc2_sample_buf[ADC_SAMPLE_ONE_GRUP_NUM];
    uint32_t sensor2SwitchCurrentMoment; // Unit: ms
    float sensor3;
    average_filter_t sensor3Filter;
    uint16_t adc3_sample_buf[ADC_SAMPLE_ONE_GRUP_NUM];
    uint32_t sensor3SwitchCurrentMoment; // Unit: ms
    float sensor4;
    average_filter_t sensor4Filter;
    uint16_t adc4_sample_buf[ADC_SAMPLE_ONE_GRUP_NUM];
    uint32_t sensor4SwitchCurrentMoment; // Unit: ms
    float sensor5;
    average_filter_t sensor5Filter;
    uint16_t adc5_sample_buf[ADC_SAMPLE_ONE_GRUP_NUM];
    uint32_t sensor5SwitchCurrentMoment; // Unit: ms
    float ref;
    average_filter_t refFilter;
    uint16_t ref_sample_buf[ADC_SAMPLE_ONE_GRUP_NUM];
    uint32_t refSwitchCurrentMoment; // Unit: ms
};

void SysMeasureInit();

/**
 * @brief
 *
 * @param data
 * @return true 有数据
 * @return false 没有数据
 */
bool SysMeasureRead(struct SysMeasureData_t *data);
#define ADC_ID1 1
#define ADC_ID2 2
#ifdef __cplusplus
}
#endif
#endif //! SYSMEASURE_H
