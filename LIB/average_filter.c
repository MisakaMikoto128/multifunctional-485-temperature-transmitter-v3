/**
 * @file average_filter.c
 * @author Liu Yuanlin (liuyuanlins@outlook.com)
 * @brief
 * @version 0.1
 * @date 2024-01-23
 * @last modified 2024-01-23
 *
 * @copyright Copyright (c) 2024 Liu Yuanlin Personal.
 *
 */
#include "average_filter.h"
#include <stdlib.h>
#include <string.h>
#include <math.h>
bool average_filter_new(average_filter_t *af, uint8_t size)

{
    af->count = 0;
    af->index = 0;
    af->size  = size;
    af->pBuf  = (float *)malloc(size * sizeof(float));
    if (af->pBuf == NULL) {
        return false;
    }
    for (int i = 0; i < size; i++) {
        af->pBuf[i] = 0;
    }
    af->sum       = 0;
    af->average   = 0;
    af->maxVar    = 0;
    af->minVar    = 0;
    af->middleVar = 0;

    return true;
}

bool average_filter_delete(average_filter_t *af)
{
    if (af->pBuf != NULL) {
        free(af->pBuf);
        af->pBuf = NULL;
        return true;
    } else {
        return false;
    }
}

/**
 * @file average_filter.c
 * @brief Implementation of the average filter update function.
 */

/**
 * @brief Updates the average filter with a new value.
 *
 * This function updates the average filter with a new value and returns the updated average.
 * The average filter calculates the average value of a series of input values and keeps track of the maximum and minimum variances.
 *
 * @param af The average filter object.
 * @param value The new value to be added to the filter.
 * @return The updated average value.
 */
float average_filter_update(average_filter_t *af, float value)
{
    // Update the sum by subtracting the oldest value and adding the new value
    af->sum += value - af->pBuf[af->index];

    // Update the buffer with the new value
    af->pBuf[af->index] = value;

    // Update the index
    af->index = (af->index + 1) % af->size;

    // Update the count
    if (af->count < af->size) {
        af->count++;
    }

    // Calculate the average
    af->average = af->sum / af->count;

    // Calculate the maximum variance
    af->maxVar = 0;
    for (int i = 0; i < af->count; i++) {
        af->maxVar = fmaxf(af->maxVar, fabsf(af->pBuf[i]));
    }

    // Calculate the minimum variance
    af->minVar = af->maxVar;
    for (int i = 0; i < af->count; i++) {
        af->minVar = fminf(af->minVar, fabsf(af->pBuf[i]));
    }

    // Calculate the middle variance
    af->middleVar = (af->maxVar + af->minVar) / 2;

    return af->average;
}

void average_filter_reset(average_filter_t *af)
{
    af->count = 0;
    af->index = 0;
    af->sum   = 0;
    for (int i = 0; i < af->size; i++) {
        af->pBuf[i] = 0;
    }
    af->average   = 0;
    af->maxVar    = 0;
    af->minVar    = 0;
    af->middleVar = 0;
}