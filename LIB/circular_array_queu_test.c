#include <stdio.h>
#include "circular_array_queu.h"

CircularArrayQueue_t queue;
uint8_t queue_buf[1024] = {0};
int circular_array_queu_test()
{
    uint8_t arr[10] = {0};
    c_arr_queue_create(&queue, queue_buf, sizeof(queue_buf));
    printf("[CircularArrayQueue Test]:size : %d\n", c_arr_queue_size(&queue));
    c_arr_queue_in(&queue, "1234", 4);
    c_arr_queue_enqueue(&queue, "5");
    int cnt = 0;

    cnt += c_arr_queue_out(&queue, arr, 10);
    arr[cnt] = 0;
    printf("[CircularArrayQueue Test]:%s\n", arr);
    printf("[CircularArrayQueue Test]:dequeue size : %d\n", cnt);
    printf("[CircularArrayQueue Test]:size : %d\n", c_arr_queue_size(&queue));
    c_arr_queue_in(&queue, "1234", 4);
    printf("[CircularArrayQueue Test]:size : %d\n", c_arr_queue_size(&queue));
    c_arr_queue_enqueue(&queue, "5");
    printf("[CircularArrayQueue Test]:size : %d\n", c_arr_queue_size(&queue));
    c_arr_queue_dequeue(&queue, arr);
    printf("[CircularArrayQueue Test]:size : %d\n", c_arr_queue_size(&queue));
    return 0;
}