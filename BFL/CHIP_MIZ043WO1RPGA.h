/**
 * @file CHIP_MIZ043WO1RPGA.h
 * @author Liu Yuanlin (liuyuanlins@outlook.com)
 * @brief
 * @version 0.1
 * @date 2024-03-28
 * @last modified 2024-03-28
 *
 * @copyright Copyright (c) 2024 Liu Yuanlin Personal.
 *
 */
#ifndef CHIP_MIZ043WO1RPGA_H
#define CHIP_MIZ043WO1RPGA_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stdbool.h>
#define CHIP_MIZ043WO1RPGA_WIDGETS_MAX_LEN 25 //单位：字节
void CHIP_MIZ043WO1RPGA_Init();
void CHIP_MIZ043WO1RPGA_Printf(int widgetAddress, int widgetMaxLength, const char *fmt, ...);
#ifdef __cplusplus
}
#endif
#endif //! CHIP_MIZ043WO1RPGA_H
