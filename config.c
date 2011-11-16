#include <inttypes.h>
#include <stddef.h>
#include <string.h>
#include <stdlib.h>

#include <avr/io.h>

#include "config.h"
#include "rtc.h"
#include "usart.h"
#include "iocontrol.h"


#define SERIALNUM_LENGTH		8
#define EVENTLIST_SIZE		16

struct s_config {
	uint16_t			S1Baud;
	uint16_t			S2Baud;
	int8_t			SerialNumber[SERIALNUM_LENGTH];
	uint8_t 			NumEvents;
	struct s_mucron	EventList[EVENTLIST_SIZE];
};

uint8_t EEPROM_read(uint16_t uiAddress)
{	/* Wait for completion of previous write */	while(EECR & (1<<EEPE)) ;	/* Set up address register */
	EEAR = uiAddress;
	/* Start eeprom read by writing EERE */
	EECR |= (1<<EERE);
	/* Return data from Data Register */
	return EEDR;}

void EEPROM_read_page(uint16_t start_address, uint8_t *data, uint16_t length)
{
	uint16_t index = 0;
	for (; index < length; ++index)
	{
		*(data + index) = EEPROM_read(start_address + index);
	}
}

void EEPROM_write(uint16_t uiAddress, uint8_t ucData)
{	/* Wait for completion of previous write */
	while(EECR & (1<<EEPE));
	
	/* Set up address and Data Registers */
	EEAR = uiAddress;
	EEDR = ucData;
	
	/* Write logical one to EEMPE */
	EECR |= (1<<EEMPE);
	/* Start eeprom write by setting EEPE */
	EECR |= (1<<EEPE);}

void EEPROM_write_page(uint16_t start_address, uint8_t *data, uint16_t length)
{
	uint16_t index = 0;
	for(;index < length; ++index)
	{
		EEPROM_write(start_address + index,  *(data + index));	
	}	
}

/* EEPROM memory is much too slow to iterate through every time a timer tick is processed.
 * Keep an event list in ram that is initialized everytime the micro starts up
 */
struct s_mucron ramEventList[EVENTLIST_SIZE];

void config_Init()
{
	EEPROM_read_page(offsetof(struct s_config, EventList), (void*)ramEventList, sizeof(struct s_mucron) * EVENTLIST_SIZE);
}

uint16 config_get_baud(uint8 port)
{
	uint16 baud = 0;
	switch(port)
	{
		case 0:
			baud = EEPROM_read(offsetof(struct s_config, S1Baud)) << 8;
			baud |= EEPROM_read(offsetof(struct s_config, S1Baud) + 1);
			break;
		case 1:
			baud = EEPROM_read(offsetof(struct s_config, S2Baud)) << 8;
			baud |= EEPROM_read(offsetof(struct s_config, S2Baud) + 1);
			break;
	}	
	return baud;
}

void config_set_baud(uint8 port, uint16 baud)
{
	switch (port)
	{
		case 0:
			EEPROM_write(offsetof(struct s_config, S1Baud), baud << 8);
			EEPROM_write(offsetof(struct s_config, S1Baud) + 1, baud);
			break;
		case 1:
			EEPROM_write(offsetof(struct s_config, S2Baud), baud << 8);
			EEPROM_write(offsetof(struct s_config, S2Baud) + 1, baud);
			break;	
	}	
}

void blank_eventlist_eeprom()
{
	char tmp[sizeof(struct s_mucron)] = {0};
	
	uint8_t index;
	for (index = 0; index < EVENTLIST_SIZE; index++)
	{
		EEPROM_write_page(offsetof(struct s_config, EventList) + (sizeof(struct s_mucron) * index), tmp, sizeof(struct s_mucron) );
	}
}

void mucron_write_mem()
{
	EEPROM_write_page(offsetof(struct s_config, EventList), (void*)ramEventList, sizeof(struct s_mucron) * EVENTLIST_SIZE);
}

void mucron_list_events()
{
	uint16_t event_index = 0;
	struct s_mucron * event_ptr;
	uint8_t number_string[16];
	
	DSEND(0, "Start Event List\n");
	
	for(; event_index < EVENTLIST_SIZE; event_index++)
	{
		event_ptr = ramEventList + event_index;
		if (event_ptr->start_time && event_ptr->on_len)
		{
			ultoa(event_index, number_string, 10);
			DSEND(0, "INDEX: ");
			DSEND(0, number_string);
			DSEND(0, "\n");
			ultoa(event_ptr->start_time, number_string, 10);
			DSEND(0, "Start time: ");
			DSEND(0, number_string);
			DSEND(0, "\n");
			
			utoa(event_ptr->on_len, number_string, 10);
			DSEND(0, "On Time: ");
			DSEND(0, number_string);
			DSEND(0, "\n");
			
			utoa(event_ptr->off_len, number_string, 10);
			DSEND(0, "Off Time: ");
			DSEND(0, number_string);
			DSEND(0, "\n");
			
			utoa(event_ptr->port, number_string, 10);
			DSEND(0, "Port: ");
			DSEND(0, number_string);
			DSEND(0, "\n");			
		}
	}
	DSEND(0, "End Event List\n");
}
void mucron_delete_event(uint16_t event_index)
{
	struct s_mucron zerodevent = {};
	memcpy(ramEventList + event_index, &zerodevent, sizeof(struct s_mucron));
	EEPROM_write_page(offsetof(struct s_config, EventList) + sizeof(struct s_mucron) * event_index,
						(void*)(ramEventList+event_index),
						sizeof(struct s_mucron));
	DSEND(0, "Delete Event Finished\n");
}

void mucron_save_event(struct s_mucron *timerblock)
{
	uint16_t event_index = 0;
	struct s_mucron * event_ptr;

	//blank_eventlist_eeprom();
	for (; event_index < EVENTLIST_SIZE; event_index++)
	{
		event_ptr = ramEventList + event_index;
		if (!event_ptr->start_time && !event_ptr->on_len)
		{
			memcpy(event_ptr, timerblock, sizeof(struct s_mucron));
			EEPROM_write_page(offsetof(struct s_config, EventList) + sizeof(struct s_mucron) * event_index, (void*)event_ptr, sizeof(struct s_mucron));
			break;
		}
	}
	
	DSEND(0, "Save Event Finished\n");
}
void mucron_tick()
{
	uint16_t event_index = 0;
	struct s_mucron * event_ptr;
	time_t timestamp = time();
	time_t modsecs;
	for (; event_index < EVENTLIST_SIZE; event_index++)
	{
		event_ptr = ramEventList + event_index;
		if (event_ptr->start_time && event_ptr->on_len)
		{
			modsecs = (timestamp - event_ptr->start_time) % (event_ptr->on_len + event_ptr->off_len);
			if ( modsecs == 0)
			{
				// on
				iocontrol(event_ptr->port, 1);
			}
			else if (modsecs == event_ptr->on_len)
			{
				// off
				iocontrol(event_ptr->port, 0);
			}
		}
	}
}


