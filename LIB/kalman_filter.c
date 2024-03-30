/**
* @file kalman_filter.c
* @author Liu Yuanlin (liuyuanlins@outlook.com)
* @brief
* @version 0.1
* @date 2024-01-23
* @last modified 2024-01-23
*
* @copyright Copyright (c) 2024 Liu Yuanlin Personal.
*
*/
#include "kalman_filter.h"

void kalman_init(kalman_filter_t *kf, float q, float r, float p, float initial_value)
{
    kf->q = q;
    kf->r = r;
    kf->p = p;
    kf->x = initial_value;
}

void kalman_update(kalman_filter_t *kf, float measurement)
{
    // 预测
    kf->p += kf->q;

    // 更新
    kf->k = kf->p / (kf->p + kf->r);
    kf->x += kf->k * (measurement - kf->x);
    kf->p *= (1 - kf->k);
}
