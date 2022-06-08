#ifndef COMMON_H
#define COMMON_H

#include <stdint.h>
#include <stdbool.h>

#include "mcu.h"

#define BITS(a,b,c,d) 0b##a##b##c##d

extern void delay(uint32_t loops);
extern void init_watchdog(bool onoff);
extern void init_clock(void);
extern void init_clock_lc(void);

#endif
