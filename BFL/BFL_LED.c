/**
 * @file BFL_LED.c
 * @author Liu Yuanlin (liuyuanlins@outlook.com)
 * @brief
 * @version 0.1
 * @date 2022-12-13
 *
 * @copyright Copyright (c) 2022 Liu Yuanlin Personal.
 *
 */
#include "BFL_LED.h"
#include "main.h"
#include "tim.h"

#define LED_RED_Pin        LL_GPIO_PIN_1
#define LED_RED_GPIO_Port  GPIOA

#define LED1_PIN           LED_RED_Pin
#define LED1_Port          LED_RED_GPIO_Port
#define LED1_Port_CLK_EN() LL_AHB2_GRP1_EnableClock(LL_AHB2_GRP1_PERIPH_GPIOA)

#define LED_PWM_MODE       0
#define LED_PWM_MAX        1000

int16_t g_led_brightness = 0;
uint8_t g_led_direction = 0;

void BFL_LED_Init()
{
#ifdef LED_PWM_MODE
    MX_TIM5_Init();
    LL_TIM_EnableCounter(TIM5);
    // Enable PWM channel 2
    LL_TIM_CC_EnableChannel(TIM5, LL_TIM_CHANNEL_CH2);
    LL_TIM_OC_SetCompareCH2(TIM5, 0);
#else
    LL_GPIO_InitTypeDef GPIO_InitStruct = {0};
    LED1_Port_CLK_EN();
    BFL_LED_Off(LED1);
    GPIO_InitStruct.Pin        = LED1_PIN;
    GPIO_InitStruct.Mode       = LL_GPIO_MODE_OUTPUT;
    GPIO_InitStruct.Speed      = LL_GPIO_SPEED_FREQ_HIGH;
    GPIO_InitStruct.OutputType = LL_GPIO_OUTPUT_PUSHPULL;
    GPIO_InitStruct.Pull       = LL_GPIO_PULL_UP;
    LL_GPIO_Init(LED1_Port, &GPIO_InitStruct);
#endif
}

void BFL_LED_On(LED_ID_e led_id)
{
    switch (led_id) {
        case LED1:
#ifdef LED_PWM_MODE
            LL_TIM_OC_SetCompareCH2(TIM5, g_led_brightness);
#else
            LL_GPIO_ResetOutputPin(LED1_Port, LED1_PIN);
#endif
            break;
        default:
            break;
    }
}

void BFL_LED_Off(LED_ID_e led_id)
{
    switch (led_id) {
        case LED1:
#ifdef LED_PWM_MODE
            LL_TIM_OC_SetCompareCH2(TIM5, 0);
#else
            LL_GPIO_SetOutputPin(LED1_Port, LED1_PIN);
#endif
            break;
        default:
            break;
    }
}

void BFL_LED_Toggle(LED_ID_e led_id)
{
    switch (led_id) {
        case LED1:
            LL_GPIO_TogglePin(LED1_Port, LED1_PIN);
        default:
            break;
    }
}

void BFL_LED_SetBrightness(LED_ID_e led_id, uint16_t brightness)
{
    switch (led_id) {
        case LED1:
#ifdef LED_PWM_MODE
            if (brightness > LED_PWM_MAX) {
                brightness = LED_PWM_MAX;
            }
            g_led_brightness = brightness;
            LL_TIM_OC_SetCompareCH2(TIM5, brightness);
#else
            if (brightness > 0) {
                BFL_LED_On(LED1);
            } else {
                BFL_LED_Off(LED1);
            }
#endif
        default:
            break;
    }
}

void BFL_LED_IncBrightness(LED_ID_e led_id, uint16_t brightness_step)
{
    switch (led_id) {
        case LED1:
            if (g_led_direction == 0) {
                g_led_brightness += brightness_step;
                if (g_led_brightness > LED_PWM_MAX) {
                    g_led_brightness = LED_PWM_MAX;
                    g_led_direction  = 1;
                }
            }

            if (g_led_direction != 0) {
                g_led_brightness -= brightness_step;
                if (g_led_brightness <= 0) {
                    g_led_brightness = 0;
                    g_led_direction  = 0;
                }
            }

            BFL_LED_SetBrightness(LED1, g_led_brightness);
        default:
            break;
    }
}