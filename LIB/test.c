/**
 * @file test.c
 * @author Liu Yuanlin (liuyuanlins@outlook.com)
 * @brief
 * @version 0.1
 * @date 2022-11-04
 *
 * @copyright Copyright (c) 2022 Liu Yuanlin Personal.
 *
 */
#include "test.h"

/**
 * @brief 完成时间测量。
 *
 * @param loop_frq_test
 */
void test_LoopFrequencyTest_handler(LoopFrequencyTest_t *loop_frq_test)
{
    switch (loop_frq_test->state)
    {
    case LOOP_FREQUENCY_TEST_STATE_START:
        loop_frq_test->measure_start_tick = HDL_G4_CPU_Time_GetTick();
        loop_frq_test->state = LOOP_FREQUENCY_TEST_STATE_MEASURING;
        break;
    case LOOP_FREQUENCY_TEST_STATE_MEASURING:

        loop_frq_test->exe_cnt++;
        if ((HDL_G4_CPU_Time_GetTick() - loop_frq_test->measure_start_tick) >= loop_frq_test->measure_time)
        {
            loop_frq_test->freq = loop_frq_test->exe_cnt * 1.0f / loop_frq_test->measure_time;
            loop_frq_test->exe_cnt = 0;
            loop_frq_test->state = LOOP_FREQUENCY_TEST_STATE_END;
        }
        break;
    case LOOP_FREQUENCY_TEST_STATE_END:
        break;
    default:
        break;
    }
}

/**
 * @brief 显示当前循环测量测量到的循环频率。
 *
 * @param loop_frq_test
 */
void test_LoopFrequencyTest_show(LoopFrequencyTest_t *loop_frq_test, const char *prifix)
{

    ULOG_INFO("[Loop_Freq]:%s %.4fKHz", prifix, loop_frq_test->freq);
}
