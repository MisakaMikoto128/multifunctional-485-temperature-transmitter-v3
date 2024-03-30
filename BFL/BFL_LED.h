/**
 * @file BFL_LED.h
 * @author Liu Yuanlin (liuyuanlins@outlook.com)
 * @brief 
 * @version 0.1
 * @date 2022-12-13
 * 
 * @copyright Copyright (c) 2022 Liu Yuanlin Personal.
 * 
 */
#ifndef BFL_LED_H
#define BFL_LED_H

#include <stdint.h>

typedef enum
{
    LED1 = 1,
} LED_ID_e;

void BFL_LED_Init();
void BFL_LED_On(LED_ID_e led_id);
void BFL_LED_Off(LED_ID_e led_id);
void BFL_LED_Toggle(LED_ID_e led_id);
void BFL_LED_SetBrightness(LED_ID_e led_id, uint16_t brightness);
void BFL_LED_IncBrightness(LED_ID_e led_id, uint16_t brightness_step);
#endif //!BFL_LED_H