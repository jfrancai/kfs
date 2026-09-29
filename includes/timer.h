#ifndef TIMER_H
#define TIMER_H

#include "stdint.h"

#define TIMER_HZ 100

void timer_init(uint32_t hz);
uint32_t timer_ticks(void);
void timer_sleep(uint32_t ticks);

#endif
