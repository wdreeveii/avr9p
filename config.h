
#ifndef _CONFIG_H
#define _CONFIG_H

#define F_CPU 20000000UL
// Inline assembly: The nop = do nothing for one clock cycle.
#define nop()  __asm__ __volatile__("nop")
#include <inttypes.h>
#include "rtc.h"
// Define some useful types:
typedef signed char int8;
typedef uint8_t uint8;
typedef uint16_t uint16;
typedef uint32_t uint32;

#define TRUE 1
#define FALSE 0

struct s_mucron {
	time_t		start_time;
	uint16_t	on_len;
	uint16_t	off_len;
	uint8_t		port;
} mucron_t;


void config_Init();
uint16 config_get_baud(uint8 port);
void config_set_baud(uint8 port, uint16 baud);

void mucron_save_event(struct s_mucron *timerblock);
void mucron_delete_event(uint16_t event_index);
void mucron_list_events();
void mucron_tick();

#endif
