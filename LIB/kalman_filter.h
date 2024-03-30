/**
* @file kalman_filter.h
* @author Liu Yuanlin (liuyuanlins@outlook.com)
* @brief
* @version 0.1
* @date 2024-01-23
* @last modified 2024-01-23
*
* @copyright Copyright (c) 2024 Liu Yuanlin Personal.
*
*/
#ifndef KALMAN_FILTER_H
#define KALMAN_FILTER_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stdbool.h>


// 定义卡尔曼滤波器结构
typedef struct {
    float q; // 过程噪声协方差
    float r; // 测量噪声协方差
    float x; // 估计值
    float p; // 估计协方差
    float k; // 卡尔曼增益
} kalman_filter_t;

void kalman_init(kalman_filter_t *kf, float q, float r, float p, float initial_value);

void kalman_update(kalman_filter_t *kf, float measurement);


#ifdef __cplusplus
}
#endif
#endif //!KALMAN_FILTER_H
