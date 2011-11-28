#include <inttypes.h>
#include <stddef.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#include <util/atomic.h>

#include "softtimer.h"
#include "rtc.h"

#define MAXEVENTS 10

typedef struct cb_info
{
	time_t trigger;
	call_back cb;
} cb_data;

cb_data st_event_list_global[MAXEVENTS];

void soft_timer_init()
{
	memset(st_event_list_global, 0, MAXEVENTS*sizeof(cb_data));
}

int8_t set_timer(time_t trigger, call_back cb)
{
	uint8_t index;
	cb_data data;
	data.trigger = trigger;
	data.cb = cb;
	
	if (!trigger || !cb)
		return -1;
	
	ATOMIC_BLOCK(ATOMIC_RESTORESTATE)
	{
		for (index = 0; index < MAXEVENTS; index++)
		{
			if (!st_event_list_global[index].trigger)
			{
				st_event_list_global[index] = data;
				break;
			}
		}
	}
	return index;
}

void reset_timer(time_t trigger, call_back cb, uint8_t slot)
{
	cb_data data;
	data.trigger = trigger;
	data.cb = cb;
	if (!trigger || !cb || !(slot < MAXEVENTS))
		return;
		
	ATOMIC_BLOCK(ATOMIC_RESTORESTATE)
	{
		st_event_list_global[slot] = data;
	}
}

void exec_nonatomic(call_back cb)
{
	sei();
	(*cb)();
	cli();
}

void soft_timer_tick()
{
	uint8_t index;
	time_t current_time = time();
	
	ATOMIC_BLOCK(ATOMIC_RESTORESTATE)
	{
		for (index = 0; index < MAXEVENTS; index++)
		{
			//printf("%lu\n", st_event_list_global[index].trigger);
			if (st_event_list_global[index].trigger == current_time)
			{
				st_event_list_global[index].trigger = 0;
				exec_nonatomic(st_event_list_global[index].cb);
			}
		}
	}
}