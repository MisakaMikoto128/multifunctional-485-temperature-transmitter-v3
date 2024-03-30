/**
 * @file scheduler.c
 * @author Liu Yuanlin (liuyuanlins@outlook.com)
 * @brief
 * @version 0.1
 * @date 2022-11-08
 *
 * @copyright Copyright (c) 2022 Liu Yuanlin Personal.
 *
 */
#include "scheduler.h"
#include "HDL_G4_CPU_Time.h"
#include "log.h"
#include "mytime.h"
#include "HDL_G4_RTC.h"

#define SCHEDULER_GET_TICK() HDL_G4_CPU_Time_GetTick()
// #define SCHEDULER_GET_TICK() ((uint32_t)HDL_G4_RTC_GetMsTimestamp())
static struct sc_list _gSchedulerList = {NULL, NULL};
static struct sc_list *gSchedulerList = &_gSchedulerList;
static float oen_tick_time = 1; // ms

/*
这个调度器原理：
如果exe_cnt < exe_times
    那么每执行完成一次任务，exe_cnt++。
否则
    当当前tick - last_exe_tick > period执行任务，即时间到了就执行被任务
    不管任务是否成功都会开始下一次计时，也就是更新last_exe_tick

一般来说last_exe_tick在第一次注册后分两种情况：
    第一次初始化那么last_exe_tick为0，如果cpu已经运行超过一秒(或者tick溢出后超过1s)
    复用之前注册过的task，且距离上次取消注册超过一秒(或者tick溢出后超过1s)
那么到scheduler_handler中处理相应的task时会马上就执行一次。
*/

bool Functional_execute(Functional_t *functional)
{
    bool ret = false;

    if (functional->fun != NULL)
    {
        ret = functional->fun(functional->arg);
    }
    return ret;
}

/**
 * @brief Scheduler初始化。
 *
 */
void scheduler_init()
{
    sc_list_init(gSchedulerList);
}

/**
 * @brief scheduler处理器。
 * 只有保证scheduler_handler执行频率大于scheduler计时器的计时分辨率的频率才能保证时间相对准确的定时任务调度。
 */
void scheduler_handler()
{
    struct sc_list *it = NULL;
    SchedulerTask_t *task = NULL;

    sc_list_foreach(gSchedulerList, it)
    {
        task = sc_list_entry(it, SchedulerTask_t, next);
        if (SCHEDULER_GET_TICK() >= (task->register_tick + task->delay_before_first_exe))
        {
            // 执行次数控制
            if (task->_exe_cnt < task->exe_times)
            {
                // 这里一定是>=，如果是 > ，那么在1 cpu tick间隔的时候时间上是2cpu tick执行一次（如果task->last_exe_tick = current_tick）。
                // 这里不允许period为0，不然就会失去调度作用。
                // 这里需要保证一定的实时性
                if ((SCHEDULER_GET_TICK() - task->last_exe_tick) >= task->period)
                {
                    bool ret = false;
                    ret = Functional_execute(&task->fun);
                    if (ret == true)
                    {
                        uint32_t current_tick = SCHEDULER_GET_TICK();
                        task->_elapsed_tick_since_last_exe = current_tick - task->last_exe_tick;
                        task->_exe_tick_error = task->_elapsed_tick_since_last_exe - task->period;

                        if (task->_exe_tick_error < task->period)
                        {
                            // TODO:这里的判断存在一定的问题
                            task->last_exe_tick += task->period;
                        }
                        else
                        {
                            task->last_exe_tick = current_tick;
                            LOG(LOG_WARN, "[Scheduler] Calibration scheduling time");
                        }

                        task->_exe_cnt++;
                        task->_exe_cnt = task->_exe_cnt == SCHEDULER_EXE_TIMES_INF ? 0 : task->_exe_cnt;
                    }
                    else
                    {
                        // 1ms补充轮询一次
                        task->last_exe_tick = SCHEDULER_GET_TICK() - task->period - 1;
                    }
                }
            }
        }
    }
}

/**
 * @brief 将任务节点注册到scheduler中。
 * @note 这个函数会避免period为0的情况，如果period为0，那么period会变为1。
 * @param sche_node
 * @retval true 注册成功，false 注册失败。
 */
bool scheduler_register(SchedulerTask_t *task)
{
    bool ret = false;
    if (task != NULL)
    {
        task->register_tick = SCHEDULER_GET_TICK();
        task->period = task->period == 0 ? 1 : task->period;
        sc_list_init(&task->next);
        sc_list_add_tail(gSchedulerList, &task->next);
        ret = true;
    }
    return ret;
}

/**
 * @brief 任务是否是注册的状态。
 *
 * @param sche_node
 * @retval true 任然是注册的，false 未注册。
 */
bool scheduler_is_task_registered(SchedulerTask_t *task)
{
    bool ret = false;
    if (task != NULL)
    {
        struct sc_list *it = NULL;
        SchedulerTask_t *_task = NULL;
        struct sc_list *item, *tmp;

        sc_list_foreach_safe(gSchedulerList, tmp, it)
        {
            _task = sc_list_entry(it, SchedulerTask_t, next);
            if (_task == task)
            {
                ret = true;
                break;
            }
        }
    }
    return ret;
}

/**
 * @brief 取消已经注册到scheduler中的任务。
 * @note 这个方法会清空已经执行的次数。
 * @param task
 * @return true 如果任务注册过，且无其他原因导致取消注册失败。
 * @return false 任务未注册过，或者其他原因取消注册成功。
 */
bool scheduler_unregister(SchedulerTask_t *task)
{
    bool ret = false;
    if (task != NULL)
    {
        struct sc_list *it = NULL;
        SchedulerTask_t *_task = NULL;
        struct sc_list *item, *tmp;

        sc_list_foreach_safe(gSchedulerList, tmp, it)
        {
            _task = sc_list_entry(it, SchedulerTask_t, next);
            if (_task == task)
            {
                // 清空task
                task->_exe_cnt = 0;

                sc_list_del(gSchedulerList, &_task->next);
                ret = true;
                break;
            }
        }
    }
    return ret;
}

/**
 * @brief 重置任务执行次数。
 *
 * @param task
 */
void scheduler_reset_exe_cnt(SchedulerTask_t *task)
{
    if (task != NULL)
    {
        task->_exe_cnt = 0;
    }
}

/**
 * @brief 设置任务节点的频率。
 *
 * @param task
 * @param freq 1-1000Hz
 * @return None
 */
void scheduler_set_freq(SchedulerTask_t *task, int freq)
{
    if (task != NULL)
    {
        task->period = 1000.0f / freq / oen_tick_time;
    }
}

Period_t period_last_exe_tick_table[MAX_PERIOD_ID + 1] = {0};

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
bool period_query(uint8_t period_id, uint32_t period)
{
    bool ret = false;

    // 这里一定是>=，如果是 > ，那么在1 cpu tick间隔的时候时间上是2cpu tick执行一次。
    // 这里不允许period为0，不然就会失去调度作用。
    if ((SCHEDULER_GET_TICK() - period_last_exe_tick_table[period_id]) >= period)
    {
        period_last_exe_tick_table[period_id] = SCHEDULER_GET_TICK();
        ret = true;
    }
    return ret;
}

/**
 * @brief 同period_query_user，只是时间记录再一个uint32_t*指向的变量中。
 *
 * @param period_recorder 记录运行时间的变量的指针。
 * @param period 周期。
 * @return true 周期到了
 * @return false 周期未到。
 */
bool period_query_user(uint32_t *period_recorder, uint32_t period)
{
    bool ret = false;
    // 这里一定是>=，如果是 > ，那么在1 cpu tick间隔的时候时间上是2cpu tick执行一次。
    // 这里不允许period为0，不然就会失去调度作用。
    if ((SCHEDULER_GET_TICK() - *period_recorder) >= period)
    {
        *period_recorder = SCHEDULER_GET_TICK();
        ret = true;
    }
    return ret;
}