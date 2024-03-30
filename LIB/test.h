/**
 * @file test.h
 * @author Liu Yuanlin (liuyuanlins@outlook.com)
 * @brief
 * @version 0.1
 * @date 2022-11-04
 *
 * @copyright Copyright (c) 2022 Liu Yuanlin Personal.
 *
 */

#ifndef TEST_H_
#define TEST_H_
#include "log.h"
#include "HDL_G4_CPU_Time.h"
#include <stdint.h>

#define LOOP_FREQUENCY_TEST_STATE_START 0
#define LOOP_FREQUENCY_TEST_STATE_MEASURING 1
#define LOOP_FREQUENCY_TEST_STATE_END 2
typedef struct tagLoopFrequencyTest_t
{
    float freq; //频率测试的结果,KHz
    //测量相关变量
    uint32_t exe_cnt;            //开始测试到结束测试loop循环次数
    uint32_t measure_start_tick; //开始测试的时刻，单位ms
    uint32_t measure_time;       //测量周期，测量多长时间计算一次频率，单位ms
    uint8_t state;               //测量状态机
} LoopFrequencyTest_t;

void test_LoopFrequencyTest_handler(LoopFrequencyTest_t *loop_frq_test);
void test_LoopFrequencyTest_show(LoopFrequencyTest_t *loop_frq_test,const char * prifix);
#define test_LoopFrequencyTest_readable(loop_frq_test) ((loop_frq_test)->state == LOOP_FREQUENCY_TEST_STATE_END ? 1 : 0)
#define test_LoopFrequencyTest_reset(loop_frq_test) ((loop_frq_test)->state = LOOP_FREQUENCY_TEST_STATE_START)

/*************************Document****************************/

/*

int main()
{
    //...

    LoopFrequencyTest_t loop_frq_test = {
        .measure_time = 1000, //测试1秒钟
        //其他成员默认初始化为0.
    };

    while (1)
    {
        //...some other handler
        test_LoopFrequencyTest_handler(&loop_frq_test);

        //放在主线程的循环里直接运行的话，打印的频率和测量周期同步
        if (test_LoopFrequencyTest_readable(&loop_frq_test))
        {
            test_LoopFrequencyTest_show(&loop_frq_test,"Main");

            //需要reset之后才能进行下一次测量。
            test_LoopFrequencyTest_reset(&loop_frq_test);
        }
    }
}

*/
/*************************Document End************************/
#endif // !TEST_H_