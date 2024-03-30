/**
 * @file scheduler.h
 * @author Liu Yuanlin (liuyuanlins@outlook.com)
 * @brief
 * @version 0.1
 * @date 2022-10-24
 *
 * @copyright Copyright (c) 2022 Liu Yuanlin Personal.
 *
 */

#ifndef SCHEDULER_H
#define SCHEDULER_H
#ifdef __cplusplus
extern "C" {
#endif
#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include <limits.h>
#include "sc_list.h"

typedef bool (*Function_t)(void *arg);

typedef struct tagFunctional
{
    Function_t fun;
    void *arg;
} Functional_t;

#define SCHEDULER_EXE_TIMES_INF UINT_MAX

typedef struct tagSchedulerTask
{
    /*上一次执行的CPU时间戳：每次执行的时候或者第一次启动任务的时候更新。
    当当前CPU tick >= last_exe_tick+exe_times时执行，之后更新last_exe_tick。
    */
    uint32_t last_exe_tick;//上一次调度并且执行了的时刻
    uint32_t exe_times; //需要执行多少次：
    uint32_t _exe_cnt;   //执行了多少次
    uint32_t period;    //执行周期，单位为CPU tick的周期，必须大于0，否则就是相当于死循环了。
    uint32_t delay_before_first_exe;//从注册第一次真正执行调度的延迟
    uint32_t register_tick;//注册时刻
    Functional_t fun;   //执行的方法
    struct sc_list next;


    int32_t _exe_tick_error;//执行时刻误差
    uint32_t _elapsed_tick_since_last_exe;//距离上传执行逝去了多少事件
} SchedulerTask_t;

/**
 * @brief Scheduler初始化。
 *
 */
void scheduler_init();

/**
 * @brief scheduler处理器。
 * 只有保证scheduler_handler执行频率大于scheduler计时器的计时分辨率的频率才能保证时间相对准确的定时任务调度。
 */
void scheduler_handler();

/**
 * @brief 将任务节点注册到scheduler中。
 *
 * @param sche_node
 * @retval true 注册成功，false 注册失败。
 */
bool scheduler_register(SchedulerTask_t *task);


/**
 * @brief 任务是否是注册的状态。
 *
 * @param sche_node
 * @retval true 任然是注册的，false 未注册。
 */
bool scheduler_is_task_registered(SchedulerTask_t *task);

/**
 * @brief 取消已经注册到scheduler中的任务。
 *
 * @param task
 * @return true 如果任务注册过，且无其他原因导致取消注册失败。
 * @return false 任务未注册过，或者其他原因取消注册成功。
 */
bool scheduler_unregister(SchedulerTask_t *task);

/**
 * @brief 重置任务执行次数。
 * 
 * @param task 
 */
void scheduler_reset_exe_cnt(SchedulerTask_t *task);
/**
 * @brief 设置任务节点的频率。
 * 
 * @param task 
 * @param freq 1-1000Hz
 * @return None 
 */
void scheduler_set_freq(SchedulerTask_t *task,int freq);

typedef uint32_t Period_t;
#define MAX_PERIOD_ID 10 //最大的周期ID号，从0开始计数。

/**
 * @brief 查询是否到了需要的周期。这个函数中高速查询，如果判断周期到了，就会
 * 返回true，否则返回false,并且当周期到了之后会更新last_exe_tick，保证每周期只会判
 * 断结果为真一次。用于在主循环中方便的构建周期性执行的代码段。
 *
 * 内置一个Period_t的最后一次执行时间的时间戳表，period_id标识。
 * @param period_id 周期id，全局唯一。
 * @param period 周期。
 * @return true 周期到了
 * @return false 周期未到。
 */
bool period_query(uint8_t period_id, uint32_t period);
/**
 * @brief 同period_query_user，只是时间记录再一个uint32_t*指向的变量中。
 *
 * @param period_recorder 记录运行时间的变量的指针。
 * @param period 周期。
 * @return true 周期到了
 * @return false 周期未到。
 */
bool period_query_user(uint32_t* period_recorder, uint32_t period);
#ifdef __cplusplus
}
#endif
#endif //! SCHEDULER_H