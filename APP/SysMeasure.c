/**
 * @file SysMeasure.c
 * @author Liu Yuanlin (liuyuanlins@outlook.com)
 * @brief
 * @version 0.1
 * @date 2024-01-23
 * @last modified 2024-01-23
 *
 * @copyright Copyright (c) 2024 Liu Yuanlin Personal.
 *
 */
#include "SysMeasure.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>
#include "main.h"
#include "adc.h"
#include "usart.h"
#include "tim.h"
#include "asyn_sys.h"
#include "HDL_G4_CPU_Time.h"
#include "BFL_RefferenceResistance.h"
#include "APP_Main.h"
#include "CHIP_MCP4017T_104.h"
#include "CHIP_TCA9548A.h"
#include "CHIP_TMP102.h"
#include "BFL_Debug.h"
#include "ntc_resistance.h"

void SysMeasurePoll();
void SysMeasureCalibration();

static bool adc1_finished = false;
static bool adc2_finished = false;
static bool adc3_finished = false;
static bool adc4_finished = false;
static bool adc5_finished = false;

static bool sysmeasure_finished                = false;
static struct SysMeasureData_t sysmeasure_data = {0};
// 0: 采集参考电压 1: 采集传感器电压
static int measure_stage                                = 0;
static int measure_stage2                               = 0;
static int measure_stage3_list[5]                       = {0};
static uint32_t switchToGetRefCurrentModeMoment         = 0;
static uint32_t switchToGetRefCurrentModeMoment_list[5] = {0};
static uint32_t switchWaitTime[5]                       = {0};
extern SysInfo_t sysinfo;

struct MCP4017T_104_Chip_t g_mcp4017t_104_chip_list[MCP4017T_104_NUM];
uint8_t g_mcp4017t_i2c_channel_map[MCP4017T_104_NUM] = {1, 5, 4, 3, 2, 0};

float sensor_calibration_list[MCP4017T_104_NUM] = {1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f}; // sensor1 sensor2 sensor3 sensor4 sensor5 ref

float g_voltage_ref_resistance_map[4] = {0};
float g_ref_current_list[128]         = {0};
#define MAX(a, b) ((a) > (b) ? (a) : (b))

void SysMeasureCalibration()
{
    HAL_ADCEx_Calibration_Start(&hadc1, ADC_SINGLE_ENDED);
    HAL_ADCEx_Calibration_Start(&hadc2, ADC_SINGLE_ENDED);
    HAL_ADCEx_Calibration_Start(&hadc3, ADC_SINGLE_ENDED);
    HAL_ADCEx_Calibration_Start(&hadc4, ADC_SINGLE_ENDED);
    HAL_ADCEx_Calibration_Start(&hadc5, ADC_SINGLE_ENDED);
}

void SysMeasureStart()
{
    HAL_TIM_Base_Start(&htim3);
    HAL_ADC_Start_DMA(&hadc1, (uint32_t *)&sysmeasure_data.adc1_sample_buf[0], ADC_SAMPLE_ONE_GRUP_NUM);
    HAL_ADC_Start_DMA(&hadc2, (uint32_t *)&sysmeasure_data.adc2_sample_buf[0], ADC_SAMPLE_ONE_GRUP_NUM);
    HAL_ADC_Start_DMA(&hadc3, (uint32_t *)&sysmeasure_data.adc3_sample_buf[0], ADC_SAMPLE_ONE_GRUP_NUM);
    HAL_ADC_Start_DMA(&hadc4, (uint32_t *)&sysmeasure_data.adc4_sample_buf[0], ADC_SAMPLE_ONE_GRUP_NUM * 2);
    HAL_ADC_Start_DMA(&hadc5, (uint32_t *)&sysmeasure_data.adc5_sample_buf[0], ADC_SAMPLE_ONE_GRUP_NUM);
    __HAL_TIM_SetCounter(&htim3, 0);
    HAL_TIM_Base_Start(&htim3);
}

#define STAGE_1_CURRENT 70

void SysMeasureInit()
{

    RefferenceResistanceInit();

#define __AVE_FILTER_LEN 20 //__AVE_FILTER_LEN = 20时，200K采样频率，DMA 200个采样点一次中断，再次平均20次，即为一个1/50周期内的平均值
    average_filter_new(&sysmeasure_data.adc1Filter, __AVE_FILTER_LEN);
    average_filter_new(&sysmeasure_data.adc2Filter, __AVE_FILTER_LEN);
    average_filter_new(&sysmeasure_data.adc3Filter, __AVE_FILTER_LEN);
    average_filter_new(&sysmeasure_data.adc4_0Filter, __AVE_FILTER_LEN);
    average_filter_new(&sysmeasure_data.adc5Filter, __AVE_FILTER_LEN);
    average_filter_new(&sysmeasure_data.adc4_1Filter, __AVE_FILTER_LEN);

    // 延时1s等待ADC电压稳定
    // HDL_G4_CPU_Time_DelayMs(1000);
    SysMeasureCalibration();
    SysMeasureStart();
    asyn_sys_register(SysMeasurePoll);

    sysmeasure_data.resistor1 = 0;
    sysmeasure_data.resistor2 = 0;
    sysmeasure_data.resistor3 = 0;
    sysmeasure_data.resistor4 = 0;
    sysmeasure_data.resistor5 = 0;

    sysmeasure_data.resistor1_last = 0;
    sysmeasure_data.resistor2_last = 0;
    sysmeasure_data.resistor3_last = 0;
    sysmeasure_data.resistor4_last = 0;
    sysmeasure_data.resistor5_last = 0;

    for (size_t i = 0; i < sizeof(g_ref_current_list) / sizeof(g_ref_current_list[0]); i++) {
        g_ref_current_list[i] = 0;
    }

    // 25摄氏度时测量的一组参考值，这里设定一组初始值，然后让计算参考电流值的过程一直进行，就不需要等待了
    // 0.877644658 10s wiper 4
    // 0.175052196 10s wiper 5
    // 0.0380995721 10s wiper 33
    // 0.0185455307 10s wiper 127
    // 0.0214072242 85
    // 0.0208791867 90
    // 0.0247132182 70
    g_ref_current_list[4]   = 0.877644658f;
    g_ref_current_list[5]   = 0.175052196f;
    g_ref_current_list[33]  = 0.0380995721f;
    g_ref_current_list[70]  = 0.0247132182f;
    g_ref_current_list[85]  = 0.0214072242f;
    g_ref_current_list[90]  = 0.0208791867f;
    g_ref_current_list[127] = 0.0185455307f;

    // 电流源电阻值初始化
    for (int i = 0; i < MCP4017T_104_NUM; i++) {
        CHIP_TCA9548A_SelectChannel(g_mcp4017t_i2c_channel_map[i]);
        CHIP_MCP4017T_104_SetWiper(&g_mcp4017t_104_chip_list[i], MCP4017T_104_WIPER_MAX);
    }
}

uint8_t wiper_setting     = 0;
uint8_t wiper_setting_bak = 0;
// 50K 33 1.90V 85
// 10K 5 1.75V
// 2K 4 1.776
// 这颗电流源芯片设定的最大输出电流越大最终输出的最大电压越大，在输出电流最小时输出的最大电压测量为2.47V,保守取值1.976V
static void Test_Poll()
{
    if (wiper_setting != wiper_setting_bak) {
        wiper_setting_bak = wiper_setting;
        for (int i = 0; i < MCP4017T_104_NUM; i++) {
            CHIP_TCA9548A_SelectChannel(g_mcp4017t_i2c_channel_map[i]);
            CHIP_MCP4017T_104_SetWiper(&g_mcp4017t_104_chip_list[i], wiper_setting);
        }
    }

    RefferenceResistanceSelect(RREF1_RESISTANCE_50K);

    g_voltage_ref_resistance_map[RefferenceResistanceGetCurrent()] = sysmeasure_data.ref;
    g_ref_current_list[g_mcp4017t_104_chip_list[5].wiper_setting]  = g_voltage_ref_resistance_map[RefferenceResistanceGetCurrent()] / RefferenceResistanceValueGetCurrent();
}

/**
 * @brief
 *
 * @param totalResistor
 * @param totalResistorLast
 * @param mcp4017t_104_chip_channel
 * @return 1 电流源电流大小发生变化
 * @return 0
 */
int calculateWiperSetting(float *totalResistor, float *totalResistorLast, float *pSensor, bool *pIsT1Connected, float *pResistorNtc, float *pTemp, uint8_t mcp4017t_104_chip_channel)
{
    uint8_t wiper_setting_cal = 0;
    int ret                   = 0;

    if (measure_stage3_list[mcp4017t_104_chip_channel] == 1) {
        if (HDL_G4_CPU_Time_GetTick() - switchToGetRefCurrentModeMoment_list[mcp4017t_104_chip_channel] > switchWaitTime[mcp4017t_104_chip_channel]) {
            measure_stage3_list[mcp4017t_104_chip_channel] = 0;
        }
    } else {

        float sensor_calibrated = *pSensor * sensor_calibration_list[mcp4017t_104_chip_channel];

        *totalResistor  = sensor_calibrated / g_ref_current_list[g_mcp4017t_104_chip_list[mcp4017t_104_chip_channel].wiper_setting];
        *pIsT1Connected = *pSensor < 2.35f;

        if (*pIsT1Connected) {
            if (*totalResistor > 89) {
                wiper_setting_cal = MCP4017T_104_WIPER_MAX;
            } else if (*totalResistor < 80 && *totalResistor > 20) {
                wiper_setting_cal = STAGE_1_CURRENT;
            } else if (*totalResistor < 10) {
                wiper_setting_cal = 5;
            } else {
                wiper_setting_cal = g_mcp4017t_104_chip_list[mcp4017t_104_chip_channel].wiper_setting;
            }

            *totalResistorLast = *totalResistor;

            if (wiper_setting_cal != g_mcp4017t_104_chip_list[mcp4017t_104_chip_channel].wiper_setting) {
                CHIP_TCA9548A_SelectChannel(g_mcp4017t_i2c_channel_map[mcp4017t_104_chip_channel]);
                CHIP_MCP4017T_104_SetWiper(&g_mcp4017t_104_chip_list[mcp4017t_104_chip_channel], wiper_setting_cal);

                ret = 1;

                measure_stage3_list[mcp4017t_104_chip_channel]                  = 1;
                switchToGetRefCurrentModeMoment_list[mcp4017t_104_chip_channel] = HDL_G4_CPU_Time_GetTick(); // ms
                switchWaitTime[mcp4017t_104_chip_channel]                       = 1100;                      // 等待电容充电时间
            } else {
                *pResistorNtc = 200 * *totalResistor / (200 - *totalResistor);
                *pTemp        = resistanceToTemperature_B3950(*pResistorNtc);
            }
        } else {
            // 此时可以矫正sensor 1,2,3,4更具sensor 5
            if (mcp4017t_104_chip_channel < 5 && sysinfo.isT5Connected == false) {
                // sensor_calibration_list[mcp4017t_104_chip_channel] = sysmeasure_data.sensor5 / *pSensor;
            }
        }
    }

    return ret;
}

void SysMeasurePoll1()
{
    Test_Poll();
}

void SysMeasurePoll()
{

    if (adc1_finished || adc2_finished || adc3_finished || adc4_finished || adc5_finished) {

        switch (measure_stage) {
            case 0:
                RefferenceResistanceSelect(RREF1_RESISTANCE_2K);
                CHIP_TCA9548A_SelectChannel(g_mcp4017t_i2c_channel_map[5]);
                CHIP_MCP4017T_104_SetWiper(&g_mcp4017t_104_chip_list[5], 4);
                switchToGetRefCurrentModeMoment = HDL_G4_CPU_Time_GetTick(); // ms
                measure_stage                   = 1;
                break;
            case 1:
                if (HDL_G4_CPU_Time_GetTick() - switchToGetRefCurrentModeMoment > 10000) {
                    measure_stage = 2;
                }
                break;
            case 2:
                g_voltage_ref_resistance_map[RefferenceResistanceGetCurrent()] = sysmeasure_data.ref;
                g_ref_current_list[g_mcp4017t_104_chip_list[5].wiper_setting]  = g_voltage_ref_resistance_map[RefferenceResistanceGetCurrent()] / RefferenceResistanceValueGetCurrent();
                measure_stage                                                  = 3;
                break;
            case 3:
                RefferenceResistanceSelect(RREF1_RESISTANCE_10K);
                CHIP_TCA9548A_SelectChannel(g_mcp4017t_i2c_channel_map[5]);
                CHIP_MCP4017T_104_SetWiper(&g_mcp4017t_104_chip_list[5], 5);
                switchToGetRefCurrentModeMoment = HDL_G4_CPU_Time_GetTick(); // ms
                measure_stage                   = 4;
                break;
            case 4:
                if (HDL_G4_CPU_Time_GetTick() - switchToGetRefCurrentModeMoment > 10000) {
                    measure_stage = 5;
                }
                break;
            case 5:
                g_voltage_ref_resistance_map[RefferenceResistanceGetCurrent()] = sysmeasure_data.ref;
                g_ref_current_list[g_mcp4017t_104_chip_list[5].wiper_setting]  = g_voltage_ref_resistance_map[RefferenceResistanceGetCurrent()] / RefferenceResistanceValueGetCurrent();
                measure_stage                                                  = 6;
                break;
            case 6:
                RefferenceResistanceSelect(RREF1_RESISTANCE_50K);
                CHIP_TCA9548A_SelectChannel(g_mcp4017t_i2c_channel_map[5]);
                CHIP_MCP4017T_104_SetWiper(&g_mcp4017t_104_chip_list[5], STAGE_1_CURRENT);
                switchToGetRefCurrentModeMoment = HDL_G4_CPU_Time_GetTick(); // ms
                measure_stage                   = 7;
                break;
            case 7:
                if (HDL_G4_CPU_Time_GetTick() - switchToGetRefCurrentModeMoment > 10000) {
                    measure_stage = 8;
                }
                break;
            case 8:
                g_voltage_ref_resistance_map[RefferenceResistanceGetCurrent()] = sysmeasure_data.ref;
                g_ref_current_list[g_mcp4017t_104_chip_list[5].wiper_setting]  = g_voltage_ref_resistance_map[RefferenceResistanceGetCurrent()] / RefferenceResistanceValueGetCurrent();
                measure_stage                                                  = 9;
                break;

            case 9:
                RefferenceResistanceSelect(RREF1_RESISTANCE_50K);
                CHIP_TCA9548A_SelectChannel(g_mcp4017t_i2c_channel_map[5]);
                CHIP_MCP4017T_104_SetWiper(&g_mcp4017t_104_chip_list[5], MCP4017T_104_WIPER_MAX);
                switchToGetRefCurrentModeMoment = HDL_G4_CPU_Time_GetTick(); // ms
                measure_stage                   = 10;
                break;
            case 10:
                if (HDL_G4_CPU_Time_GetTick() - switchToGetRefCurrentModeMoment > 10000) {
                    measure_stage = 11;
                }
                break;
            case 11:
                g_voltage_ref_resistance_map[RefferenceResistanceGetCurrent()] = sysmeasure_data.ref;
                g_ref_current_list[g_mcp4017t_104_chip_list[5].wiper_setting]  = g_voltage_ref_resistance_map[RefferenceResistanceGetCurrent()] / RefferenceResistanceValueGetCurrent();
                measure_stage                                                  = 0;
                break;
            default:
                break;
        }

        switch (measure_stage2) {
            case 0:
                // 电流源电阻值初始化
                for (int i = 0; i < MCP4017T_104_NUM - 1; i++) {
                    CHIP_TCA9548A_SelectChannel(g_mcp4017t_i2c_channel_map[i]);
                    CHIP_MCP4017T_104_SetWiper(&g_mcp4017t_104_chip_list[i], MCP4017T_104_WIPER_MAX);
                }

                switchToGetRefCurrentModeMoment = HDL_G4_CPU_Time_GetTick(); // ms
                measure_stage2                  = 1;
                break;
            case 1:
                if (HDL_G4_CPU_Time_GetTick() - switchToGetRefCurrentModeMoment > 1000) {
                    measure_stage2 = 2;
                }
                break;
            case 2:
                if (adc1_finished) {
                    calculateWiperSetting(&sysmeasure_data.resistor1, &sysmeasure_data.resistor1_last, &sysmeasure_data.sensor1, &sysinfo.isT1Connected, &sysmeasure_data.resistor1_ntc, &sysinfo.Temp1, 0);
                }

                if (adc2_finished) {
                    calculateWiperSetting(&sysmeasure_data.resistor2, &sysmeasure_data.resistor2_last, &sysmeasure_data.sensor2, &sysinfo.isT2Connected, &sysmeasure_data.resistor2_ntc, &sysinfo.Temp2, 1);
                }

                if (adc3_finished) {
                    calculateWiperSetting(&sysmeasure_data.resistor3, &sysmeasure_data.resistor3_last, &sysmeasure_data.sensor3, &sysinfo.isT3Connected, &sysmeasure_data.resistor3_ntc, &sysinfo.Temp3, 2);
                }

                if (adc4_finished) {
                    calculateWiperSetting(&sysmeasure_data.resistor4, &sysmeasure_data.resistor4_last, &sysmeasure_data.sensor4, &sysinfo.isT4Connected, &sysmeasure_data.resistor4_ntc, &sysinfo.Temp4, 3);
                }

                if (adc5_finished) {
                    calculateWiperSetting(&sysmeasure_data.resistor5, &sysmeasure_data.resistor5_last, &sysmeasure_data.sensor5, &sysinfo.isT5Connected, &sysmeasure_data.resistor5_ntc, &sysinfo.Temp5, 4);
                }
                break;
            default:
                break;
        }

        if (adc1_finished) {
            adc1_finished = false;
        }

        if (adc2_finished) {
            adc2_finished = false;
        }

        if (adc3_finished) {
            adc3_finished = false;
        }

        if (adc4_finished) {
            adc4_finished = false;
        }

        if (adc5_finished) {
            adc5_finished = false;
        }

        sysmeasure_finished = true;
    }
}

static void calculateADCDMAData(uint16_t *dataBuf, average_filter_t *pFilter, bool *pFinished)
{
    uint32_t sum = 0;
    for (int i = 0; i < ADC_SAMPLE_ONE_GRUP_NUM; i++) {
        sum += dataBuf[i];
    }
    float avg = sum * 1.0f / ADC_SAMPLE_ONE_GRUP_NUM;
    average_filter_update(pFilter, avg);
    *pFinished = true;
}

/**
 * @brief  Conversion complete callback in non-blocking mode.
 * @param hadc ADC handle
 * @retval None
 */
void HAL_ADC_ConvCpltCallback(ADC_HandleTypeDef *hadc)
{
    if (hadc->Instance == ADC1) {

        uint16_t *dataBuf         = sysmeasure_data.adc1_sample_buf;
        average_filter_t *pFilter = &sysmeasure_data.adc1Filter;
        bool *pFinished           = &adc1_finished;
        calculateADCDMAData(dataBuf, pFilter, pFinished);

        sysmeasure_data.sensor2 = pFilter->average * (VREF_VOLT / 4096);

    } else if (hadc->Instance == ADC2) {

        uint16_t *dataBuf         = sysmeasure_data.adc2_sample_buf;
        average_filter_t *pFilter = &sysmeasure_data.adc2Filter;
        bool *pFinished           = &adc2_finished;
        calculateADCDMAData(dataBuf, pFilter, pFinished);

        sysmeasure_data.sensor3 = pFilter->average * (VREF_VOLT / 4096);

    } else if (hadc->Instance == ADC3) {

        uint16_t *dataBuf         = sysmeasure_data.adc3_sample_buf;
        average_filter_t *pFilter = &sysmeasure_data.adc3Filter;
        bool *pFinished           = &adc3_finished;
        calculateADCDMAData(dataBuf, pFilter, pFinished);

        sysmeasure_data.sensor4 = pFilter->average * (VREF_VOLT / 4096);

    } else if (hadc->Instance == ADC4) {
        BFL_Debug_SignalPinSet();

        // 这个ADC有两个通道，第一个通道为sensor5-ADC4-IN4，第二个通道为sensor1-ADC4-IN5
        for (size_t i = 0; i < ADC_SAMPLE_ONE_GRUP_NUM; i++) {
            sysmeasure_data.adc4_0_sample_buf[i] = sysmeasure_data.adc4_sample_buf[i * 2];
            sysmeasure_data.adc4_1_sample_buf[i] = sysmeasure_data.adc4_sample_buf[i * 2 + 1];
        }

        calculateADCDMAData(sysmeasure_data.adc4_0_sample_buf, &sysmeasure_data.adc4_0Filter, &adc4_finished);
        calculateADCDMAData(sysmeasure_data.adc4_1_sample_buf, &sysmeasure_data.adc4_1Filter, &adc4_finished);

        sysmeasure_data.sensor1 = sysmeasure_data.adc4_1Filter.average * (VREF_VOLT / 4096);
        sysmeasure_data.sensor5 = sysmeasure_data.adc4_0Filter.average * (VREF_VOLT / 4096);

        BFL_Debug_SignalPinReset();

    } else if (hadc->Instance == ADC5) {

        uint16_t *dataBuf         = sysmeasure_data.adc5_sample_buf;
        average_filter_t *pFilter = &sysmeasure_data.adc5Filter;
        bool *pFinished           = &adc5_finished;

        sysmeasure_data.ref = pFilter->average * (VREF_VOLT / 4096);
        calculateADCDMAData(dataBuf, pFilter, pFinished);
    }
}

bool SysMeasureRead(struct SysMeasureData_t *data)
{
    if (sysmeasure_finished) {
        sysmeasure_finished = false;
        *data               = sysmeasure_data;
        return true;
    }
    return false;
}

/*
如果在i=0时就进入HardFault_Handler，那么可能的原因是adc_sample_buf指针的对齐问题。在许多硬件平台上，特别是ARM架构的平台，对于uint32_t类型的数据，其地址必须是4的倍数，这被称为数据对齐。如果试图访问一个不满足对齐要求的地址，硬件会产生一个对齐异常，导致程序进入HardFault_Handler。
在你的代码中，adc_sample_buf是一个uint32_t指针，指向adc_data_buf。adc_data_buf是一个uint8_t数组，其地址可能不是4的倍数。当你试图通过adc_sample_buf访问adc_data_buf时，如果adc_data_buf的地址不是4的倍数，就会产生对齐异常。
你可以通过以下方式检查adc_data_buf的地址是否满足对齐要求：
如果adc_data_buf的地址不满足对齐要求，你需要调整adc_data_buf的地址，使其满足对齐要求。这通常可以通过调整adc_data_buf的声明来实现，例如使用__attribute__((aligned(4)))：
这将确保adc_data_buf的地址是4的倍数，满足uint32_t类型的对齐要求。

在大多数现代操作系统和C库实现中，malloc函数返回的内存地址总是适当对齐的，可以用于任何数据类型。这是因为malloc知道可能会在分配的内存上存储任何类型的数据，所以它会返回一个适当对齐的地址，以满足最严格的对齐要求。
在具体实现中，malloc通常会返回一个地址，该地址是最大基本对齐类型（通常是double或long long）的倍数。这意味着，如果你在malloc返回的内存上存储uint32_t类型的数据，你不需要担心对齐问题。
然而，如果你在malloc返回的内存上存储了一个结构体，而该结构体的对齐要求比malloc的对齐要求更严格，那么你可能需要使用aligned_alloc函数来分配内存，以确保满足对齐要求。
总的来说，如果你使用malloc来分配内存，并在该内存上存储uint32_t类型的数据，你不应该遇到对齐问题。
*/

int g_current_refference_resistance = RREF1_RESISTANCE_NONE;

void RefferenceResistanceInit()
{
    LL_GPIO_InitTypeDef GPIO_InitStruct = {0};
    LL_AHB2_GRP1_EnableClock(LL_AHB2_GRP1_PERIPH_GPIOA);

    /**/
    LL_GPIO_ResetOutputPin(RSW3_GPIO_Port, RSW3_Pin);

    /**/
    LL_GPIO_ResetOutputPin(RSW2_GPIO_Port, RSW2_Pin);

    /**/
    LL_GPIO_ResetOutputPin(RSW1_GPIO_Port, RSW1_Pin);

    /**/
    GPIO_InitStruct.Pin        = RSW3_Pin;
    GPIO_InitStruct.Mode       = LL_GPIO_MODE_OUTPUT;
    GPIO_InitStruct.Speed      = LL_GPIO_SPEED_FREQ_MEDIUM;
    GPIO_InitStruct.OutputType = LL_GPIO_OUTPUT_PUSHPULL;
    GPIO_InitStruct.Pull       = LL_GPIO_PULL_NO;
    LL_GPIO_Init(RSW3_GPIO_Port, &GPIO_InitStruct);

    /**/
    GPIO_InitStruct.Pin        = RSW2_Pin;
    GPIO_InitStruct.Mode       = LL_GPIO_MODE_OUTPUT;
    GPIO_InitStruct.Speed      = LL_GPIO_SPEED_FREQ_MEDIUM;
    GPIO_InitStruct.OutputType = LL_GPIO_OUTPUT_PUSHPULL;
    GPIO_InitStruct.Pull       = LL_GPIO_PULL_NO;
    LL_GPIO_Init(RSW2_GPIO_Port, &GPIO_InitStruct);

    /**/
    GPIO_InitStruct.Pin        = RSW1_Pin;
    GPIO_InitStruct.Mode       = LL_GPIO_MODE_OUTPUT;
    GPIO_InitStruct.Speed      = LL_GPIO_SPEED_FREQ_MEDIUM;
    GPIO_InitStruct.OutputType = LL_GPIO_OUTPUT_PUSHPULL;
    GPIO_InitStruct.Pull       = LL_GPIO_PULL_NO;
    LL_GPIO_Init(RSW1_GPIO_Port, &GPIO_InitStruct);

    RefferenceResistanceSelect(RREF1_RESISTANCE_NONE);
}

/**
 * @brief
 *
 */
void RefferenceResistanceSelect(int Resistance_ID)
{
    switch (Resistance_ID) {
        case RREF1_RESISTANCE_NONE:
            LL_GPIO_ResetOutputPin(RSW3_GPIO_Port, RSW3_Pin);
            LL_GPIO_ResetOutputPin(RSW2_GPIO_Port, RSW2_Pin);
            LL_GPIO_ResetOutputPin(RSW1_GPIO_Port, RSW1_Pin);
            g_current_refference_resistance = RREF1_RESISTANCE_NONE;
            break;
        case RREF1_RESISTANCE_50K:
            LL_GPIO_ResetOutputPin(RSW3_GPIO_Port, RSW3_Pin);
            LL_GPIO_ResetOutputPin(RSW2_GPIO_Port, RSW2_Pin);
            LL_GPIO_SetOutputPin(RSW1_GPIO_Port, RSW1_Pin);
            g_current_refference_resistance = RREF1_RESISTANCE_50K;
            break;
        case RREF1_RESISTANCE_10K:
            LL_GPIO_ResetOutputPin(RSW3_GPIO_Port, RSW3_Pin);
            LL_GPIO_SetOutputPin(RSW2_GPIO_Port, RSW2_Pin);
            LL_GPIO_ResetOutputPin(RSW1_GPIO_Port, RSW1_Pin);
            g_current_refference_resistance = RREF1_RESISTANCE_10K;
            break;
        case RREF1_RESISTANCE_2K:
            LL_GPIO_ResetOutputPin(RSW3_GPIO_Port, RSW3_Pin);
            LL_GPIO_SetOutputPin(RSW2_GPIO_Port, RSW2_Pin);
            LL_GPIO_SetOutputPin(RSW1_GPIO_Port, RSW1_Pin);
            g_current_refference_resistance = RREF1_RESISTANCE_2K;
            break;
        default:
            break;
    }
}

int RefferenceResistanceGetCurrent()
{
    return g_current_refference_resistance;
}

int RefferenceResistanceValueGetCurrent()
{
    switch (g_current_refference_resistance) {
        case RREF1_RESISTANCE_NONE:
            return -1;
            break;
        case RREF1_RESISTANCE_50K:
            return 50;
            break;
        case RREF1_RESISTANCE_10K:
            return 10;
            break;
        case RREF1_RESISTANCE_2K:
            return 2;
            break;
        default:
            return 0;
            break;
    }
}