#include "asyn_sys.h"
#include <stddef.h>
#define ASYN_SYS_MAX_FUNCS 10
static AsynSys_Functional_t _gAsynSysFuncs[ASYN_SYS_MAX_FUNCS] = {0};
/**
 * @brief 注册异步系统函数。如果函数已经存在，则不做任何操作。
 * 
 * @param func 
 */
void asyn_sys_register(AsynSys_Func_t func)
{
    for (size_t i = 0; i < ASYN_SYS_MAX_FUNCS; i++)
    {
        if (_gAsynSysFuncs[i].func == func)
        {
            return;
        }
    }
    for (size_t i = 0; i < ASYN_SYS_MAX_FUNCS; i++)
    {
        if (_gAsynSysFuncs[i].func == NULL)
        {
            _gAsynSysFuncs[i].func = func;
            return;
        }
    }
}
/**
 * @brief 注销异步系统函数。如果函数不存在，则不做任何操作。
 * 
 * @param func 
 */
void asyn_sys_unregister(AsynSys_Func_t func)
{
    for (size_t i = 0; i < ASYN_SYS_MAX_FUNCS; i++)
    {
        if (_gAsynSysFuncs[i].func == func)
        {
            _gAsynSysFuncs[i].func = NULL;
            return;
        }
    }
}

/**
 * @brief 轮询异步系统函数。
 * 
 */
void asyn_sys_poll()
{
    for (size_t i = 0; i < ASYN_SYS_MAX_FUNCS; i++)
    {
        if (_gAsynSysFuncs[i].func != NULL)
        {
            _gAsynSysFuncs[i].func();
        }
    }
}