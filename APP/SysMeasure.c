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

void SysMeasurePoll();
void SysMeasureCalibration();

static bool __IO adc1_finished = false;
static bool __IO adc2_finished = false;
static bool __IO adc3_finished = false;
static bool __IO adc4_finished = false;
static bool __IO adc5_finished = false;

static bool sysmeasure_finished                = false;
static struct SysMeasureData_t sysmeasure_data = {0};
// 0: 采集参考电压 1: 采集传感器电压
static int measure_stage                        = 0;
static uint32_t switchToGetRefCurrentModeMoment = 0;
extern SysInfo_t sysinfo;

struct MCP4017T_104_Chip_t g_mcp4017t_104_chip_list[MCP4017T_104_NUM];
uint8_t g_mcp4017t_i2c_channel_map[MCP4017T_104_NUM] = {1, 5, 4, 3, 2, 0};


float g_voltage_ref_resistance_map[4] = {0};

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
    HAL_ADC_Start_DMA(&hadc4, (uint32_t *)&sysmeasure_data.adc4_sample_buf[0], ADC_SAMPLE_ONE_GRUP_NUM);
    HAL_ADC_Start_DMA(&hadc5, (uint32_t *)&sysmeasure_data.adc5_sample_buf[0], ADC_SAMPLE_ONE_GRUP_NUM);
    __HAL_TIM_SetCounter(&htim3, 0);
    HAL_TIM_Base_Start(&htim3);
}

void SysMeasureInit()
{

    RefferenceResistanceInit();

#define __AVE_FILTER_LEN 20 //__AVE_FILTER_LEN = 20时，200K采样频率，DMA 200个采样点一次中断，再次平均20次，即为一个1/50周期内的平均值
    average_filter_new(&sysmeasure_data.sensor1Filter, __AVE_FILTER_LEN);
    average_filter_new(&sysmeasure_data.sensor2Filter, __AVE_FILTER_LEN);
    average_filter_new(&sysmeasure_data.sensor3Filter, __AVE_FILTER_LEN);
    average_filter_new(&sysmeasure_data.sensor4Filter, __AVE_FILTER_LEN);
    average_filter_new(&sysmeasure_data.sensor5Filter, __AVE_FILTER_LEN);
    average_filter_new(&sysmeasure_data.refFilter, __AVE_FILTER_LEN);

    // 延时1s等待ADC电压稳定
    // HDL_G4_CPU_Time_DelayMs(1000);
    SysMeasureCalibration();
    SysMeasureStart();
    asyn_sys_register(SysMeasurePoll);
}

void SysMeasurePoll()
{

    static int stage = 0;
    if (adc1_finished && adc2_finished && adc3_finished && adc4_finished && adc5_finished) {
        adc1_finished = false;
        adc2_finished = false;
        adc3_finished = false;
        adc4_finished = false;
        adc5_finished = false;

        sysmeasure_finished = true;

        switch (stage) {
            case 0:
                RefferenceResistanceSelect(RREF1_RESISTANCE_50K);
                CHIP_TCA9548A_SelectChannel(g_mcp4017t_i2c_channel_map[5]);
                CHIP_MCP4017T_104_SetWiper(&g_mcp4017t_104_chip_list[5], MCP4017T_104_WIPER_MAX);
                break;
            case 1:
                break;
            default:
                break;
        }
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
        average_filter_t *pFilter = &sysmeasure_data.sensor1Filter;
        bool *pFinished           = &adc1_finished;
        calculateADCDMAData(dataBuf, pFilter, pFinished);

        sysmeasure_data.sensor1 = pFilter->average;
        HAL_ADC_Stop_DMA(&hadc1);

    } else if (hadc->Instance == ADC2) {

        uint16_t *dataBuf         = sysmeasure_data.adc2_sample_buf;
        average_filter_t *pFilter = &sysmeasure_data.sensor2Filter;
        bool *pFinished           = &adc2_finished;
        calculateADCDMAData(dataBuf, pFilter, pFinished);

        sysmeasure_data.sensor2 = pFilter->average;
        HAL_ADC_Stop_DMA(&hadc2);

    } else if (hadc->Instance == ADC3) {

        uint16_t *dataBuf         = sysmeasure_data.adc3_sample_buf;
        average_filter_t *pFilter = &sysmeasure_data.sensor3Filter;
        bool *pFinished           = &adc3_finished;
        calculateADCDMAData(dataBuf, pFilter, pFinished);

        sysmeasure_data.sensor3 = pFilter->average;
        HAL_ADC_Stop_DMA(&hadc3);

    } else if (hadc->Instance == ADC4) {

        uint16_t *dataBuf         = sysmeasure_data.adc4_sample_buf;
        average_filter_t *pFilter = &sysmeasure_data.sensor4Filter;
        bool *pFinished           = &adc4_finished;

        sysmeasure_data.sensor4 = pFilter->average;
        calculateADCDMAData(dataBuf, pFilter, pFinished);

        switch (measure_stage) {
            case 0:
                // 采集参考电压
                g_voltage_ref_resistance_map[RefferenceResistanceGetCurrent()] = sysmeasure_data.sensor4;
                break;
            case 1:
                // 采集传感器电压
                break;
            default:
                break;
        }

        HAL_ADC_Stop_DMA(&hadc4);

    } else if (hadc->Instance == ADC5) {

        uint16_t *dataBuf         = sysmeasure_data.adc5_sample_buf;
        average_filter_t *pFilter = &sysmeasure_data.sensor5Filter;
        bool *pFinished           = &adc5_finished;

        sysmeasure_data.sensor5 = pFilter->average;
        calculateADCDMAData(dataBuf, pFilter, pFinished);

        HAL_ADC_Stop_DMA(&hadc5);
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