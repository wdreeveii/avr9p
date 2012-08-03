#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <util/atomic.h>

#include "config.h"
#include "9p.h"
#include "usart.h"
#include "rtc.h"
#include "softtimer.h"

int8_t build_baud(uint8_t port, uint8_t *reply, uint8_t size)
{
	uint8_t * replyptr = reply + 4;
	
	uint8_t outlength = 0;
	*((uint32_t *)(reply)) = 0;
		
	size -= 4;
	switch(config_get_baud(port))
	{
		case 1041:
			outlength = strlcpy((char*)replyptr, "2400\n", size);
			break;
		case 520:
			outlength = strlcpy((char*)replyptr, "4800\n", size);
			break;
		case 259:
			outlength = strlcpy((char*)replyptr, "9600\n", size);
			break;
		case 173:
			outlength = strlcpy((char*)replyptr, "14400\n", size);
			break;
		case 129:
			outlength = strlcpy((char*)replyptr, "19200\n", size);
			break;
		case 86:
			outlength = strlcpy((char*)replyptr, "28800\n", size);
			break;
		case 64:
			outlength = strlcpy((char*)replyptr, "38400\n", size);
			break;
		case 42:
			outlength = strlcpy((char*)replyptr, "57600\n", size);
			break;
		case 32:
			outlength = strlcpy((char*)replyptr, "76800\n", size);
			break;
		case 21:
			outlength = strlcpy((char*)replyptr, "115200\n", size);
			break;
		case 10:
			outlength = strlcpy((char*)replyptr, "230400\n", size);
			break;
		case 9:
			outlength = strlcpy((char*)replyptr, "250000\n", size);
			break;
		case 4:
			outlength = strlcpy((char*)replyptr, "500000\n", size);
			break;
	}
	*((uint32_t *)(reply)) = outlength;
	replyptr += outlength;
	return replyptr - reply;
}
int16_t usart0_read_baud(uint8_t oc, const struct DirectoryEntry *dp, uint16_t tag, uint64_t * offset, uint32_t * count)
{
	uint8_t reply[20];
	uint8_t length = 4;
	
	if (*(offset) == 0)
		length = build_baud(0, reply, 20);
	
	p9_send_reply(oc, Rread, tag, reply, length);
	return 0;
}
int16_t usart1_read_baud(uint8_t oc, const struct DirectoryEntry *dp, uint16_t tag, uint64_t * offset, uint32_t * count)
{
	uint8_t reply[20];
	uint8_t length = 4;
	
	if (*(offset) == 0)
		length = build_baud(1, reply, 20);

	p9_send_reply(oc, Rread, tag, reply, length);
	return 0;
}
int16_t encode_baud(uint32_t stdbaud)
{
	switch(stdbaud)
	{
		case 2400: return 1041;
		case 4800: return 520;
		case 9600: return 259;
		case 14400: return 173;
		case 19200: return 129;
		case 28800: return 86;
		case 38400: return 64;
		case 57600: return 42;
		case 76800: return 32;
		case 115200: return 21;
		case 230400: return 10;
		case 250000: return 9;
		case 500000: return 4;
		default: return -1;
	}
	return -1;
}
int16_t usart0_baud = -1;
void usart0_do_write()
{
	ATOMIC_BLOCK(ATOMIC_RESTORESTATE)
	{
		if (usart0_baud >= 0)
		{
			USART_Init0(usart0_baud);
			usart0_baud = -1;
		}
	}
}

int16_t usart1_baud = -1;
void usart1_do_write()
{
	ATOMIC_BLOCK(ATOMIC_RESTORESTATE)
	{
		if (usart1_baud >= 0)
		{
			USART_Init1(usart1_baud);
			usart1_baud = -1;
		}
	}
}

int16_t usart0_write_baud(const struct DirectoryEntry *dp, uint64_t *offset, uint32_t *count, uint8_t *indata)
{
	uint8_t datacpy[*count + 1];
	datacpy[*count] = 0;
	uint32_t newbaud;
	int16_t avrbaud;
	memcpy(datacpy, indata, *count);
	if (sscanf((char *)datacpy, "%lu", &newbaud) == 1)
	{
		avrbaud = encode_baud(newbaud);
		if (avrbaud >= 0)
		{
			ATOMIC_BLOCK(ATOMIC_RESTORESTATE)
			{
				usart0_baud = avrbaud;
				set_timer(time() + 1, &usart0_do_write);
			}
			config_set_baud(0, avrbaud);
		}
		else return -1;
	}
	else return -1;
	return *count;
}
int16_t usart1_write_baud(const struct DirectoryEntry *dp, uint64_t *offset, uint32_t *count, uint8_t *indata)
{
	uint8_t datacpy[*count + 1];
	datacpy[*count] = 0;
	uint32_t newbaud;
	int16_t avrbaud;
	memcpy(datacpy, indata, *count);
	if (sscanf((char *)datacpy, "%lu", &newbaud) == 1)
	{
		avrbaud = encode_baud(newbaud);
		if (avrbaud >= 0)
		{
			ATOMIC_BLOCK(ATOMIC_RESTORESTATE)
			{
				usart0_baud = avrbaud;
				set_timer(time() + 1, &usart1_do_write);
			}
			config_set_baud(1, avrbaud);
		}
		else return -1;
	}
	else return -1;
	return *count;
}

#define MUCRONLISTLINESIZE 37
const char mucronlistheader[] = "Index	Start Time	On Time	Off Time Port\n";
int16_t mucron_list_events(uint8_t oc, const struct DirectoryEntry *dp, uint16_t tag, uint64_t * offset, uint32_t * count)
{
	uint8_t reply[*count + 4];
	uint8_t *replyptr = reply + 4;
	char line[35];
	uint8_t statsize = 0;
	uint8_t copylen;
	uint16_t event_index = 0;
	struct s_mucron * event_ptr;
	
	statsize = sizeof(mucronlistheader) - 1;
	// -1 because sizeof returns size of array not length of string
	if (*offset < statsize)
	{
		copylen= strlcpy((char *)replyptr, mucronlistheader + *offset, *count);
		*count -= copylen;
		*offset += copylen;
		replyptr += copylen;
	}

	for(event_ptr = mucron_get_eventlist(); event_index < MUCRON_EVENTLIST_SIZE; event_ptr++, event_index++)
	{
		if (*count == 0)
			break;
		if (event_ptr->start_time && event_ptr->on_len)
		{
			copylen = sprintf(line, "%u	%lu	%lu	%lu	 %u\n", event_index, event_ptr->start_time,event_ptr->on_len,event_ptr->off_len,event_ptr->port);
			if (*offset > statsize + copylen)
			{
				statsize += copylen;
				continue;
			}
			copylen = strlcpy((char *)replyptr, line + (*offset - (statsize)), *count);
			*count -= copylen;
			*offset += copylen;
			replyptr += copylen;
			statsize += copylen;
		}
	}
	*((uint32_t *)reply) = replyptr - reply - 4;
	p9_send_reply(oc, Rread,tag,reply,replyptr - reply);
	return 0;
}

int16_t mucron_build_save_event(const struct DirectoryEntry *dp, uint64_t *offset, uint32_t *count, uint8_t *indata)
{
	// need to null terminate the string in order to use sscanf... easiest way to do this
	uint8_t datacpy[*count + 1];
	datacpy[*count] = 0;
	uint16_t portcpy; // sscanf will stuff 2 bytes so have to handle that here
	struct s_mucron data;

	memcpy(datacpy, indata, *count);
	
	if (sscanf((char *)datacpy, "%lu %lu %lu %u",(uint32_t *) &data.start_time, &data.on_len, &data.off_len, &portcpy) == 4)
	{
		data.port = portcpy;
		mucron_save_event(&data);
	}
	else return -1;
	return *count;
}
int16_t mucron_build_delete_event(const struct DirectoryEntry *dp, uint64_t *offset, uint32_t *count, uint8_t *indata)
{
	uint8_t datacpy[*count + 1];
	datacpy[*count] = 0;
	uint16_t index;
	memcpy(datacpy, indata, *count);
	if (sscanf((char *)datacpy, "%u", &index) == 1)
	{
		mucron_delete_event(index);
	}
	else return -1;
	return *count;
}

extern uint8_t timer_paused;

int16_t mucron_is_paused(uint8_t oc, const struct DirectoryEntry *dp, uint16_t tag, uint64_t * offset, uint32_t * count)
{
	uint8_t reply[20];
	uint8_t len = 0;
	uint8_t timer_copy;
	
	if (*(offset) == 0)
	{
		ATOMIC_BLOCK(ATOMIC_RESTORESTATE)
		{
			timer_copy = timer_paused;
		}
		
		if (timer_copy)
			*((uint32_t *)reply) = len = sprintf((char *)(reply + 4), "Yes");
		else
			*((uint32_t *)reply) = len = sprintf((char *)(reply + 4), "No");
	}
	p9_send_reply(oc, Rread, tag, reply, len+4);
	return 0;
}

int16_t mucron_pause_event(const struct DirectoryEntry *dp, uint64_t *offset, uint32_t *count, uint8_t *indata)
{
	uint8_t datacpy[*count + 1];
	datacpy[*count] = 0;
	uint16_t state = 0;
	memcpy(datacpy, indata, *count);
	if (sscanf((char *)datacpy, "%u", &state) == 1)
	{
		ATOMIC_BLOCK(ATOMIC_RESTORESTATE)
		{
			timer_paused = state;
		}
	}
	else return -1;
	return *count;
}

int16_t rtc_write_clock(const struct DirectoryEntry *dp, uint64_t *offset, uint32_t *count, uint8_t *indata)
{
	uint8_t datacpy[*count + 1];
	datacpy[*count] = 0;
	int32_t deltat;
	time_t newtime;
	memcpy(datacpy, indata, *count);
	if (sscanf((char *)datacpy, "%lu", &newtime) == 1)
	{
		deltat = newtime - time();
		update_timers(deltat);
		set_time(newtime);
	}
	else return -1;
	return *count;
}

int16_t rtc_read_clock(uint8_t oc, const struct DirectoryEntry *dp, uint16_t tag, uint64_t * offset, uint32_t * count)
{
	uint8_t reply[20];
	uint8_t len = 0;
	
	if (*(offset) == 0)
		*((uint32_t *)reply) = len = sprintf((char *)(reply + 4), "%lu\n", time());

	p9_send_reply(oc, Rread, tag, reply, len + 4);
	return 0;
}

extern char __bss_end;
extern void *__brkval;

int16_t freemem_read(uint8_t oc, const struct DirectoryEntry *dp, uint16_t tag, uint64_t * offset, uint32_t * count)
{
	uint8_t reply[20];
	uint8_t len = 0;
	uint16_t free_memory;
	
	if (*(offset) == 0)
	{
		if((uint16_t)__brkval == 0)
			free_memory = ((uint16_t)&free_memory) - ((uint16_t)&__bss_end);
		else
			free_memory = ((uint16_t)&free_memory) - ((uint16_t)__brkval);
		*((uint32_t *)reply) = len = sprintf((char *)(reply + 4), "%u\n", free_memory);
	}
	p9_send_reply(oc, Rread, tag, reply, len+4);
	return 0;
		
	
}

DirectoryEntry * p9_build_config_timer(uint8_t parent_qid, DirectoryEntry * parent)
{
	DirectoryEntry *dir_timer = (DirectoryEntry *)malloc(6 * sizeof(DirectoryEntry));
	if (!dir_timer)
	{
		printf("Dir build malloc fail\n");
		return 0;
	}
	*(dir_timer) = (DirectoryEntry){"..", {QTDIR, 0, parent_qid}, parent};
	*(dir_timer + 1) = (DirectoryEntry){"new_event", {QTFILE, 0, p9_register_de(dir_timer+1)}, 0, p9_noread, mucron_build_save_event};
	*(dir_timer + 2) = (DirectoryEntry){"delete_event",{QTFILE, 0, p9_register_de(dir_timer+2)}, 0, p9_noread, mucron_build_delete_event};
	*(dir_timer + 3) = (DirectoryEntry){"list", {QTFILE, 0, p9_register_de(dir_timer+3)}, 0, mucron_list_events, p9_nowrite};
	*(dir_timer + 4) = (DirectoryEntry){"pause", {QTFILE, 0, p9_register_de(dir_timer+4)}, 0, mucron_is_paused, mucron_pause_event};
	*(dir_timer + 5) = (DirectoryEntry){0};
	
	return dir_timer;
}

DirectoryEntry * p9_build_config_dir(uint8_t parent_qid, DirectoryEntry * parent)
{
	uint8_t tmpde;
	DirectoryEntry *dir_config = (DirectoryEntry *)malloc(7 * sizeof(DirectoryEntry));
	if (!dir_config)
	{
		printf("Dir build malloc fail\n");
		return 0;
	}
	*(dir_config) = (DirectoryEntry){"..", {QTDIR, 0, parent_qid}, parent};
	*(dir_config + 1) = (DirectoryEntry){"usart0_baud", {QTFILE, 0, p9_register_de(dir_config+1)}, 0, usart0_read_baud, usart0_write_baud};
	*(dir_config + 2) = (DirectoryEntry){"usart1_baud", {QTFILE, 0, p9_register_de(dir_config+2)}, 0, usart1_read_baud, usart1_write_baud};
	tmpde = p9_register_de(dir_config+3);
	*(dir_config + 3) = (DirectoryEntry){"timer", {QTDIR, 0, tmpde}, p9_build_config_timer(tmpde, dir_config)};
	*(dir_config + 4) = (DirectoryEntry){"clock", {QTFILE, 0, p9_register_de(dir_config + 4)}, 0, rtc_read_clock, rtc_write_clock};
	*(dir_config + 5) = (DirectoryEntry){"freemem", {QTFILE, 0, p9_register_de(dir_config + 5)}, 0, freemem_read, p9_nowrite};
	*(dir_config + 6) = (DirectoryEntry){0};
	
	return dir_config;
}