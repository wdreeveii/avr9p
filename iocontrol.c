#include "config.h"
#include <avr/io.h>
#include <avr/pgmspace.h>

struct io_port {
	char * regaddr;
	uint8_t bit;
};

struct io_port ports[] PROGMEM = {
	{(char *)&DDRA, 0},
	{(char *)&DDRA, 1},
	{(char *)&DDRA, 2},
	{(char *)&DDRA, 3},
	{(char *)&DDRA, 4},
	{(char *)&DDRA, 5},
	{(char *)&DDRA, 6},
	{(char *)&DDRA, 7},
	{(char *)&DDRD, 7},
	{(char *)&DDRD, 6},
	{(char *)&DDRD, 5},
	{(char *)&DDRD, 4},
	{(char *)&DDRC, 5},
	{(char *)&DDRC, 4}
};

#define NELEM(x) (sizeof(x)/sizeof(x[0]))
void iocontrol(unsigned char port, unsigned char on)
{
	if (port >= NELEM(ports))
		return;
		
	struct io_port ctrl;
	ctrl.regaddr = (char *)pgm_read_word(&(ports[port].regaddr));
	ctrl.bit = pgm_read_byte(&(ports[port].bit));
	
	if (on)
		*(ctrl.regaddr) |= (1U<<(ctrl.bit));
	else
		*(ctrl.regaddr) &= ~(1U<<(ctrl.bit));
}

void ioflip(unsigned char port)
{
	if (port >= NELEM(ports))
		return;
	struct io_port ctrl;
	ctrl.regaddr = (char *)pgm_read_word(&(ports[port].regaddr));
	ctrl.bit = pgm_read_byte(&(ports[port].bit));
	
	unsigned char state = (*(ctrl.regaddr)) & (1U<<(ctrl.bit));
	
	if (state)
		*(ctrl.regaddr) &= ~(1U<<(ctrl.bit));
	else
		*(ctrl.regaddr) |= (1U<<(ctrl.bit));
}
