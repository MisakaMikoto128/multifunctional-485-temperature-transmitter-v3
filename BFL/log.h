/**
* @file log.h
* @author Liu Yuanlin (liuyuanlins@outlook.com)
* @brief
* @version 0.1
* @date 2023-01-10
* @last modified 2023-01-10
*
* @copyright Copyright (c) 2023 Liu Yuanlin Personal.
*
*/
#ifndef LOG_H
#define LOG_H

#include "ulog.h"
void ulog_init_user();
void Debug_Printf(const void *format, ...);

#endif //!LOG_H