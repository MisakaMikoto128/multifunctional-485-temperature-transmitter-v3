#ifndef __HDL_G4_IWDG_H
#define __HDL_G4_IWDG_H

#include "stdint.h"
#include "stm32g4xx_ll_iwdg.h"

extern void HDL_G4_WATCHDOG_Init(uint16_t timeout);
extern void HDL_G4_WATCHDOG_Feed(void);

#endif
