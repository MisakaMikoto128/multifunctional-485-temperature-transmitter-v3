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
#define VREF_VOLT               3.2951f

#define FFT_POINT_NUM           (ADC_CHANNEL_NUM * ADC_SAMPLE_NUM)

#define Fs                      200000                        // Hz
#define FrqSamp                 50                            // 被采样波形的频率
#define DC_IDEX                 0                             // 变换结果中直流分量的位置
#define IDEX_50                 (50 * FFT_POINT_NUM / Fs)     // 变换结果中50Hz分量的位置
#define CIDEX_50                (50 * 2 * FFT_POINT_NUM / Fs) // 变换结果中50Hz分量的位置

#define ADC_SAMPLE_ONE_GRUP_NUM 400

#define MCP4017T_104_NUM        6

struct SysMeasureData_t {
    float sensor1;
    float sensor2;
    float sensor3;
    float sensor4;
    float sensor5;
    float ref;

    float resistor1; // Unit: kΩ
    float resistor2; // Unit: kΩ
    float resistor3; // Unit: kΩ
    float resistor4; // Unit: kΩ
    float resistor5; // Unit: kΩ

    float resistor1_ntc; // Unit: kΩ
    float resistor2_ntc; // Unit: kΩ
    float resistor3_ntc; // Unit: kΩ
    float resistor4_ntc; // Unit: kΩ
    float resistor5_ntc; // Unit: kΩ

    float resistor1_last; // Unit: kΩ
    float resistor2_last; // Unit: kΩ
    float resistor3_last; // Unit: kΩ
    float resistor4_last; // Unit: kΩ
    float resistor5_last; // Unit: kΩ

    float Temp1; // Unit: ℃
    float Temp2; // Unit: ℃
    float Temp3; // Unit: ℃
    float Temp4; // Unit: ℃
    float Temp5; // Unit: ℃

    float TempD1; // Unit: ℃

    average_filter_t adc1Filter;
    uint16_t adc1_sample_buf[ADC_SAMPLE_ONE_GRUP_NUM];
    uint32_t sensor1SwitchCurrentMoment; // Unit: ms

    average_filter_t adc2Filter;
    uint16_t adc2_sample_buf[ADC_SAMPLE_ONE_GRUP_NUM];
    uint32_t sensor2SwitchCurrentMoment; // Unit: ms

    average_filter_t adc3Filter;
    uint16_t adc3_sample_buf[ADC_SAMPLE_ONE_GRUP_NUM];
    uint32_t sensor3SwitchCurrentMoment; // Unit: ms

    average_filter_t adc4_0Filter;
    uint16_t adc4_sample_buf[ADC_SAMPLE_ONE_GRUP_NUM * 2];
    uint32_t sensor4SwitchCurrentMoment; // Unit: ms

    average_filter_t adc5Filter;
    uint16_t adc5_sample_buf[ADC_SAMPLE_ONE_GRUP_NUM];
    uint32_t sensor5SwitchCurrentMoment; // Unit: ms

    average_filter_t adc4_1Filter;
    uint16_t adc4_1_sample_buf[ADC_SAMPLE_ONE_GRUP_NUM];
    uint16_t adc4_0_sample_buf[ADC_SAMPLE_ONE_GRUP_NUM];
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
