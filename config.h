
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

#define MUCRON_EVENTLIST_SIZE		16

#define NELEM(x) (sizeof(x)/sizeof(x[0]))

struct s_mucron {
	time_t		start_time;
	uint32_t	on_len;
	uint32_t	off_len;
	uint8_t		port;
};


void config_Init();
void config_get_io_types(uint8_t *data);
void config_set_io_types(uint8_t *data);
uint8_t config_get_protocol_type(uint8_t port);
void config_set_protocol_type(uint8_t port, uint8_t type);

uint16 config_get_baud(uint8 port);
void config_set_baud(uint8 port, uint16 baud);
void blank_eventlist_eeprom();

void mucron_save_event(struct s_mucron *timerblock);
void mucron_delete_event(uint16_t event_index);
void mucron_tick();
struct s_mucron * mucron_get_eventlist();

uint8_t timer_paused;

#endif
