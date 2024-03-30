#include "circular_array_queu.h"
/**
 * @brief 创建一个静态的循环数组队列。
 *
 * @param Q 队列对象的指针。
 * @param pBuf 队列数据存放内存指针。
 * @param bufSize 队列数据存放内存大小，作为队列的容量。
 */
void c_arr_queue_create(CircularArrayQueue Q, ElementType *pBuf, uint32_t bufSize)
{
    Q->Capacity = bufSize;
    Q->pData = pBuf;

    // Make empty
    Q->Front = 0;
    Q->Rear = 0;
}

/**
 * @brief 队列是否为空。
 *
 * @param Q 队列对象的指针。
 * @return uint8_t 1为空，0不空。
 */
uint8_t c_arr_queue_is_empty(CircularArrayQueue Q)
{
    return Q->Front == Q->Rear;
}

/**
 * @brief 队列是否满了。
 *
 * @param Q
 * @return uint8_t 1为满，0不满。
 */
uint8_t c_arr_queue_is_full(CircularArrayQueue Q)
{
    return (Q->Rear + 1) % Q->Capacity == Q->Front;
}

/**
 * @brief 队列中数据的大小，单位是多少个元素，不一定是字节。
 *
 * @param Q
 * @return uint32_t 队列中数据的大小。
 */
uint32_t c_arr_queue_size(CircularArrayQueue Q)
{
    return (Q->Rear - Q->Front + Q->Capacity) % Q->Capacity;
}

/**
 * @brief 队列入队一个元素。
 *
 * @param Q
 * @param pX 指向入队元素对象的指针。
 * @return uint8_t 1入队成功，0入队失败队列满了。
 */
uint8_t c_arr_queue_enqueue(CircularArrayQueue Q, const ElementType *pX)
{
    uint8_t ret = 0;
    if (!c_arr_queue_is_full(Q))
    {
        Q->pData[Q->Rear] = *pX;
        Q->Rear = (Q->Rear + 1) % Q->Capacity;
        ret = 1;
    }
    return ret;
}

/**
 * @brief 队列出队一个元素。
 *
 * @param Q
 * @param pX 指向出队元素对象的指针。
 * @return uint8_t 1出队成功，0出队失败队列为空。
 */
uint8_t c_arr_queue_dequeue(CircularArrayQueue Q, ElementType *pX)
{
    uint8_t ret = 0;
    if (!c_arr_queue_is_empty(Q))
    {
        // font
        *pX = Q->pData[Q->Front];
        // dequeue
        Q->Front = (Q->Front + 1) % Q->Capacity;
        ret = 1;
    }

    return ret;
}

/**
 * @brief 向队列中入队一段数据。
 *
 * @param Q
 * @param pBuf 指向入队元素内存的指针。
 * @param bufSize 入队元素的个数。
 * @return uint32_t 实际入队的元素个数。
 */
uint32_t c_arr_queue_in(CircularArrayQueue Q, const ElementType *pBuf, uint32_t bufSize)
{
    uint32_t ret = 0;

    while (!c_arr_queue_is_full(Q) && bufSize > 0)
    {
        Q->pData[Q->Rear] = pBuf[ret];
        Q->Rear = (Q->Rear + 1) % Q->Capacity;
        bufSize--;
        ret++;
    }

    return ret;
}

/**
 * @brief 将队列中一部分元素出队
 *
 * @param Q
 * @param pBuf 指向出存放出队元素内存的指针。
 * @param bufSize 希望出队的元素个数。
 * @return uint32_t 实际出队的元素个数。
 */
uint32_t c_arr_queue_out(CircularArrayQueue Q, ElementType *pBuf, uint32_t bufSize)
{
    uint32_t ret = 0;

    while (!c_arr_queue_is_empty(Q) && bufSize > 0)
    {
        // font
        pBuf[ret] = Q->pData[Q->Front];
        // dequeue
        Q->Front = (Q->Front + 1) % Q->Capacity;
        bufSize--;
        ret++;
    }

    return ret;
}
