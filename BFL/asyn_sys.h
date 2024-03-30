/**
* @file asyn_sys.h
* @author Liu Yuanlin (liuyuanlins@outlook.com)
* @brief
* @version 0.1
* @date 2023-01-15
* @last modified 2023-01-15
*
* @copyright Copyright (c) 2023 Liu Yuanlin Personal.
*
*/
#ifndef ASYN_SYS_H
#define ASYN_SYS_H

typedef void (*AsynSys_Func_t)(void);

typedef struct
{
    AsynSys_Func_t func;
} AsynSys_Functional_t;

/**
 * @brief 注册异步系统函数。如果函数已经存在，则不做任何操作。
 * 
 * @param func 
 */
void asyn_sys_register(AsynSys_Func_t func);
/**
 * @brief 注销异步系统函数。如果函数不存在，则不做任何操作。
 * 
 * @param func 
 */
void asyn_sys_unregister(AsynSys_Func_t func);

/**
 * @brief 轮询异步系统函数。
 * 
 */
void asyn_sys_poll();

#endif //!ASYN_SYS_H