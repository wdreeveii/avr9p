#include "config.h"
#include <avr/io.h>

struct io_port {
	char * regaddr;
	uint8_t bit;
};

struct io_port ports[] = {
	{0, 0},
	{0, 1},
	{0, 2},
	{0, 3},
	{0, 4},
	{0, 5},
	{0, 6},
	{0, 7},
	{0, 7},
	{0, 6},
	{0, 5},
	{0, 4},
	{0, 5},
	{0, 4}
};

void ioinit ()
{
	uint8_t index = 0;
	for (; index < 8; index++)
	{
		ports[index].regaddr = (char *)&(DDRA);
	}
	for (; index < 12; index++)
	{
		ports[index].regaddr = (char *)&(DDRD);
	}
	for (; index < 14; index++)
	{
		ports[index].regaddr = (char *)&(DDRC);
	}
}
void iocontrol(unsigned char port, unsigned char on)
{
	if (on)
		*(ports[port].regaddr) |= (1U<<(ports[port].bit));
	else
		*(ports[port].regaddr) &= ~(1U<<(ports[port].bit));
}

void ioflip(unsigned char port)
{
	unsigned char state = (*(ports[port].regaddr)) & (1U<<(ports[port].bit));
	
	if (state)
		*(ports[port].regaddr) &= ~(1U<<(ports[port].bit));
	else
		*(ports[port].regaddr) |= (1U<<(ports[port].bit));
}
