#ifndef _CircularArrayQueue_h
#define _CircularArrayQueue_h

#include <stdint.h>

/*
--> rear | | | | | | | font-->
*/
#define ElementType uint8_t
struct CircularArrayQueueRecord;
typedef struct CircularArrayQueueRecord *CircularArrayQueue;

void c_arr_queue_create(CircularArrayQueue Q, ElementType *pBuf, uint32_t bufSize);
uint8_t c_arr_queue_is_empty(CircularArrayQueue Q);
uint8_t c_arr_queue_is_full(CircularArrayQueue Q);
uint32_t c_arr_queue_size(CircularArrayQueue Q);
uint8_t c_arr_queue_enqueue(CircularArrayQueue Q, const ElementType *pX);
uint8_t c_arr_queue_dequeue(CircularArrayQueue Q, ElementType *pX);
uint32_t c_arr_queue_in(CircularArrayQueue Q, const ElementType *pBuf, uint32_t bufSize);
uint32_t c_arr_queue_out(CircularArrayQueue Q, ElementType *pBuf, uint32_t bufSize);

typedef struct CircularArrayQueueRecord
{
    int Capacity;
    int Front;
    int Rear;
    ElementType * pData;
}CircularArrayQueue_t;

#endif /* _CircularArrayQueue_h */
