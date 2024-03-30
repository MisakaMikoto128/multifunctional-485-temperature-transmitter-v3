/**
 * @file HDL_G4_CPU_Time.c
 * @author Liu Yuanlin (liuyuanlins@outlook.com)
 * @brief
 * @version 0.1
 * @date 2022-10-08
 *
 * @copyright Copyright (c) 2022 Liu Yuanlin Personal.
 *
 */
#include "HDL_G4_CPU_Time.h"
static CPU_Time_Callback_t cpu_tick_callback = NULL;

#define CPU_TIM    TIM1
#define CPU_US_TIM TIM2
/**
 * 定时器：TIM1
 * 中断优先级: 抢占0，子优先级0
 * 计数周期1ms
 * 主频：170MHz
 */

__IO uint32_t uwCpuTick;

#define CPU_MAX_US_DELAY 0xFFFFFFFFU

/**
 * @brief CPU滴答时钟初始化。
 *
 */
void HDL_G4_CPU_Time_Init()
{
    LL_TIM_InitTypeDef TIM_InitStruct = {0};

    /* Peripheral clock enable */
    LL_APB2_GRP1_EnableClock(LL_APB2_GRP1_PERIPH_TIM1);

    /* TIM1 interrupt Init */
    NVIC_SetPriority(TIM1_UP_TIM16_IRQn, NVIC_EncodePriority(NVIC_GetPriorityGrouping(), 0, 0));
    NVIC_EnableIRQ(TIM1_UP_TIM16_IRQn);

    /* USER CODE BEGIN TIM1_Init 1 */

    /* USER CODE END TIM1_Init 1 */
    TIM_InitStruct.Prescaler = 17UL - 1; // CNM,SB LL libarary and stupid CUBEMX.
                                         // Work does not talk about money, research direction is not clear, graduation is difficult
    TIM_InitStruct.CounterMode       = LL_TIM_COUNTERMODE_UP;
    TIM_InitStruct.Autoreload        = 10000UL - 1;
    TIM_InitStruct.ClockDivision     = LL_TIM_CLOCKDIVISION_DIV1;
    TIM_InitStruct.RepetitionCounter = 0;
    LL_TIM_Init(TIM1, &TIM_InitStruct);

    LL_TIM_DisableARRPreload(TIM1);

    uint32_t tmpsmcr;

    /* Reset the SMS, TS, ECE, ETPS and ETRF bits */
    tmpsmcr = TIM1->SMCR;
    tmpsmcr &= ~(TIM_SMCR_SMS | TIM_SMCR_TS);
    tmpsmcr &= ~(TIM_SMCR_ETF | TIM_SMCR_ETPS | TIM_SMCR_ECE | TIM_SMCR_ETP);
    TIM1->SMCR = tmpsmcr;

    LL_TIM_SetClockSource(TIM1, LL_TIM_CLOCKSOURCE_INTERNAL);
    LL_TIM_SetTriggerOutput(TIM1, LL_TIM_TRGO_RESET);
    LL_TIM_SetTriggerOutput2(TIM1, LL_TIM_TRGO2_RESET);
    LL_TIM_DisableMasterSlaveMode(TIM1);

    LL_TIM_SetCounter(CPU_TIM, 0);
    LL_TIM_EnableIT_UPDATE(TIM1); // 更新中断使能
    LL_TIM_EnableCounter(TIM1);   // 计数使能

    // 微秒定时器初始化
    /* Peripheral clock enable */
    LL_APB1_GRP1_EnableClock(LL_APB1_GRP1_PERIPH_TIM2);

    /* USER CODE BEGIN TIM2_Init 1 */

    /* USER CODE END TIM2_Init 1 */
    TIM_InitStruct.Prescaler         = 170UL - 1;
    TIM_InitStruct.CounterMode       = LL_TIM_COUNTERMODE_UP;
    TIM_InitStruct.Autoreload        = 0xFFFFFFFFUL;
    TIM_InitStruct.ClockDivision     = LL_TIM_CLOCKDIVISION_DIV1;
    TIM_InitStruct.RepetitionCounter = 0;
    LL_TIM_Init(TIM2, &TIM_InitStruct);
    LL_TIM_DisableARRPreload(TIM2);
    LL_TIM_SetClockSource(TIM2, LL_TIM_CLOCKSOURCE_INTERNAL);
    LL_TIM_SetTriggerOutput(TIM2, LL_TIM_TRGO_RESET);
    LL_TIM_DisableMasterSlaveMode(TIM2);

    LL_TIM_SetCounter(TIM2, 0);
    LL_TIM_DisableIT_UPDATE(TIM2);

    NVIC_SetPriority(TIM2_IRQn, NVIC_EncodePriority(NVIC_GetPriorityGrouping(), 15, 0));
    NVIC_EnableIRQ(TIM2_IRQn);

    LL_TIM_EnableCounter(TIM2); // 计数使能
}

/**
 * @brief 获取CPU滴答时钟。
 * @note 这个在中断中和RTC同步了，但是与US不同步。
 * @return uint32_t
 */
uint32_t HDL_G4_CPU_Time_GetTick()
{
    return uwCpuTick;
}

/**
 * @brief 阻塞式毫秒延时方法。
 *
 * @param DelayMs 延时毫秒数。
 */
void HDL_G4_CPU_Time_DelayMs(uint32_t DelayMs)
{
    uint32_t tickstart = HDL_G4_CPU_Time_GetTick();
    uint32_t wait      = DelayMs;

    while ((HDL_G4_CPU_Time_GetTick() - tickstart) < wait) {
    }
}

/**
 * @brief 重置CPU滴答时钟。
 *
 */
void HDL_G4_CPU_Time_ResetTick()
{
    LL_TIM_SetCounter(CPU_TIM, 0);
    uwCpuTick = 0;
}

/**
 * @brief
 *
 * @return uint32_t
 */
uint32_t HDL_G4_CPU_Time_GetUsTick()
{
    return LL_TIM_GetCounter(CPU_US_TIM);
}

/**
 * @brief 重置CPU微秒滴答时钟。
 *
 */
void HDL_G4_CPU_Time_ResetUsTick()
{
    LL_TIM_SetCounter(CPU_US_TIM, 0);
}

/**
 * @brief This function provides minimum delay (in microsecond) based
 *        on variable incremented.
 * @note 这个函数使用了32bit定时器,需要注意的是Debug模式下即使停止在断点，这个
 * 定时器还是在运行。
 * @param DelayUs specifies the delay time length, in microsecond.
 * @retval None
 */
void HDL_G4_CPU_Time_DelayUs(uint32_t DelayUs)
{
    uint32_t tickstart = HDL_G4_CPU_Time_GetUsTick();
    uint32_t wait      = DelayUs;

    while ((HDL_G4_CPU_Time_GetUsTick() - tickstart) < wait) {
    }
}

/* 保存 TIM定时中断到后执行的回调函数指针 */
static CPU_Time_Callback_t s_TIM_CallBack1;
static CPU_Time_Callback_t s_TIM_CallBack2;
static CPU_Time_Callback_t s_TIM_CallBack3;
static CPU_Time_Callback_t s_TIM_CallBack4;

/**
 * @brief 使用TIM2-5做单次定时器使用, 定时时间到后执行回调函数。可以同时启动4个定时器通道，互不干扰。
 *          定时精度正负1us （主要耗费在调用本函数的执行时间）
 *          TIM2和TIM5 是32位定时器。定时范围很大
 *          TIM3和TIM4 是16位定时器。
 *
 * @param _CC : 捕获比较通道几，1，2，3, 4
 * @param _uiTimeOut : 超时时间, 单位 1us. 对于16位定时器，最大 65.5ms; 对于32位定时器，最大 4294秒
 * @param _pCallBack : 定时时间到后，被执行的函数
 * @param _pArg : 定时时间到后，被执行的函数所需要参数的地址。
 * @retval None
 */
void HDL_G4_CPU_Time_StartHardTimer(uint8_t _CC, uint32_t _uiTimeOut, void *_pCallBack)
{
    uint32_t cnt_now;
    uint32_t cnt_tar;
    TIM_TypeDef *TIMx = CPU_US_TIM;

    /* 无需补偿延迟，实测精度正负1us */
    cnt_now = TIMx->CNT;
    cnt_tar = cnt_now + _uiTimeOut; /* 计算捕获的计数器值 */
    if (_CC == 1) {
        s_TIM_CallBack1 = (CPU_Time_Callback_t)_pCallBack;
        TIMx->CCR1      = cnt_tar;               /* 设置捕获比较计数器CC1 */
        TIMx->SR        = (uint16_t)~TIM_IT_CC1; /* 清除CC1中断标志 */
        TIMx->DIER |= TIM_IT_CC1;                /* 使能CC1中断 */
    } else if (_CC == 2) {
        s_TIM_CallBack2 = (CPU_Time_Callback_t)_pCallBack;
        TIMx->CCR2      = cnt_tar;               /* 设置捕获比较计数器CC2 */
        TIMx->SR        = (uint16_t)~TIM_IT_CC2; /* 清除CC2中断标志 */
        TIMx->DIER |= TIM_IT_CC2;                /* 使能CC2中断 */
    } else if (_CC == 3) {
        s_TIM_CallBack3 = (CPU_Time_Callback_t)_pCallBack;
        TIMx->CCR3      = cnt_tar;               /* 设置捕获比较计数器CC3 */
        TIMx->SR        = (uint16_t)~TIM_IT_CC3; /* 清除CC3中断标志 */
        TIMx->DIER |= TIM_IT_CC3;                /* 使能CC3中断 */
    } else if (_CC == 4) {
        s_TIM_CallBack4 = (CPU_Time_Callback_t)_pCallBack;
        TIMx->CCR4      = cnt_tar;               /* 设置捕获比较计数器CC4 */
        TIMx->SR        = (uint16_t)~TIM_IT_CC4; /* 清除CC4中断标志 */
        TIMx->DIER |= TIM_IT_CC4;                /* 使能CC4中断 */
    } else {
        return;
    }
}

/**
 * @brief 关闭硬件定时器。实际上就是关中断。
 * 如果定时器已经执行完，那么不会改变任何寄存器。
 * 如果定时器正在执行，且关闭的时刻不再临界条件，那么会关闭中断。
 * 如果刚好CCx中断标志置位但是还没有进入中断，且此时清楚了CCx中断标志,那么会进入中断
 * 但是却不能执行回调函数。
 *
 * 这个函数主要还是用于启动定时器后不需要了，要在定时中间关闭，不打算定时器去执行回调函数的情况。
 * @param _CC : 捕获比较通道几，1，2，3, 4
 */
void HDL_G4_CPU_Time_StopHardTimer(uint8_t _CC)
{
    uint32_t cnt_now;
    uint32_t cnt_tar;
    TIM_TypeDef *TIMx = CPU_US_TIM;
    switch (_CC) {
        case 1: {
            TIMx->SR = (uint16_t)~TIM_IT_CC1;    /* 清除CC1中断标志 */
            TIMx->DIER &= (uint16_t)~TIM_IT_CC1; /* 禁能CC1中断 */
        } break;
        case 2: {
            TIMx->SR = (uint16_t)~TIM_IT_CC2;    /* 清除CC2中断标志 */
            TIMx->DIER &= (uint16_t)~TIM_IT_CC2; /* 禁能CC2中断 */
        } break;
        case 3: {
            TIMx->SR = (uint16_t)~TIM_IT_CC3;    /* 清除CC3中断标志 */
            TIMx->DIER &= (uint16_t)~TIM_IT_CC3; /* 禁能CC3中断 */
        } break;
        case 4: {
            TIMx->SR = (uint16_t)~TIM_IT_CC4;    /* 清除CC4中断标志 */
            TIMx->DIER &= (uint16_t)~TIM_IT_CC4; /* 禁能CC4中断 */
        } break;
    }
}

/**
 * @brief This function handles TIM1 update interrupt and TIM16 global interrupt.
 * @retval None
 */
void TIM1_UP_TIM16_IRQHandler(void)
{
    if (LL_TIM_IsActiveFlag_UPDATE(CPU_TIM)) {
        uwCpuTick++;
        if (cpu_tick_callback != NULL) {
            cpu_tick_callback();
        }
        LL_TIM_ClearFlag_UPDATE(CPU_TIM);
    }
}

/**
 * @brief This function handles TIM2 global interrupt.
 * @retval None
 */
void TIM2_IRQHandler()
{
    uint16_t itstatus = 0x0, itenable = 0x0;
    TIM_TypeDef *TIMx = CPU_US_TIM;

    itstatus = TIMx->SR & TIM_IT_CC1;
    itenable = TIMx->DIER & TIM_IT_CC1;

    if ((itstatus != (uint16_t)RESET) && (itenable != (uint16_t)RESET)) {
        TIMx->SR = (uint16_t)~TIM_IT_CC1;
        TIMx->DIER &= (uint16_t)~TIM_IT_CC1; /* 禁能CC1中断 */

        /* 先关闭中断，再执行回调函数。因为回调函数可能需要重启定时器 */
        s_TIM_CallBack1();
    }

    itstatus = TIMx->SR & TIM_IT_CC2;
    itenable = TIMx->DIER & TIM_IT_CC2;
    if ((itstatus != (uint16_t)RESET) && (itenable != (uint16_t)RESET)) {
        TIMx->SR = (uint16_t)~TIM_IT_CC2;
        TIMx->DIER &= (uint16_t)~TIM_IT_CC2; /* 禁能CC2中断 */

        /* 先关闭中断，再执行回调函数。因为回调函数可能需要重启定时器 */
        s_TIM_CallBack2();
    }

    itstatus = TIMx->SR & TIM_IT_CC3;
    itenable = TIMx->DIER & TIM_IT_CC3;
    if ((itstatus != (uint16_t)RESET) && (itenable != (uint16_t)RESET)) {
        TIMx->SR = (uint16_t)~TIM_IT_CC3;
        TIMx->DIER &= (uint16_t)~TIM_IT_CC3; /* 禁能CC2中断 */

        /* 先关闭中断，再执行回调函数。因为回调函数可能需要重启定时器 */
        s_TIM_CallBack3();
    }

    itstatus = TIMx->SR & TIM_IT_CC4;
    itenable = TIMx->DIER & TIM_IT_CC4;
    if ((itstatus != (uint16_t)RESET) && (itenable != (uint16_t)RESET)) {
        TIMx->SR = (uint16_t)~TIM_IT_CC4;
        TIMx->DIER &= (uint16_t)~TIM_IT_CC4; /* 禁能CC4中断 */

        /* 先关闭中断，再执行回调函数。因为回调函数可能需要重启定时器 */
        s_TIM_CallBack4();
    }
}

void HDL_G4_CPU_Time_RegisterTickCallback(CPU_Time_Callback_t callback)
{
    // TODO : interrupt protection
    cpu_tick_callback = callback;
}

/**
 * @brief 同period_query_user，只是时间记录再一个uint32_t*指向的变量中。
 *
 * @param period_recorder 记录运行时间的变量的指针。
 * @param period 周期,单位tick。
 * @return true 周期到了
 * @return false 周期未到。
 */
uint8_t period_query_user(PeriodREC_t *period_recorder, PeriodREC_t period)
{
    uint8_t ret = 0;

    // 这里一定是>=，如果是 > ，那么在1 cpu tick间隔的时候时间上是2cpu tick执行一次。
    // 这里不允许period为0，不然就会失去调度作用。
    if ((HDL_G4_CPU_Time_GetTick() - *period_recorder) >= period) {
        *period_recorder = HDL_G4_CPU_Time_GetTick();
        ret              = 1;
    }
    return ret;
}