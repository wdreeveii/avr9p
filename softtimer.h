
#ifndef _SOFTTIMER_H
#define _SOFTTIMER_H

#include <stddef.h>
#include "rtc.h"

typedef void (*call_back)();

int8_t set_timer(time_t trigger, call_back cb);
void update_timers(int32_t deltat);
void reset_timer(time_t trigger, call_back cb, uint8_t slot);
void soft_timer_tick();
void soft_timer_init();

#endif