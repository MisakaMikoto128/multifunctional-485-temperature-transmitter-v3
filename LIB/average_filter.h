/**
* @file average_filter.h
* @author Liu Yuanlin (liuyuanlins@outlook.com)
* @brief
* @version 0.1
* @date 2024-01-23
* @last modified 2024-01-23
*
* @copyright Copyright (c) 2024 Liu Yuanlin Personal.
*
*/
#ifndef AVERAGE_FILTER_H
#define AVERAGE_FILTER_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stdbool.h>

// 平均滤波
typedef struct {
    uint8_t count; // 当前滤波窗口内的数据个数
    uint8_t index; // 当前滤波窗口内的数据索引
    uint8_t size; // 滤波窗口大小
    float *pBuf;
    float sum;
    
    float average;
    float maxVar;
    float minVar;
    float middleVar;
} average_filter_t;

bool average_filter_new(average_filter_t *af, uint8_t size);
bool average_filter_delete(average_filter_t *af);
/**
 * @brief 
 * 
 * @param af 
 * @param value 
 * @return float current average value
 */
float average_filter_update(average_filter_t *af, float value);
void average_filter_reset(average_filter_t *af);

#ifdef __cplusplus
}
#endif
#endif //!AVERAGE_FILTER_H
