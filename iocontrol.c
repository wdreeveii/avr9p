#include "config.h"
#include <avr/io.h>

void iocontrol(unsigned char port, unsigned char on)
{
	if (on)
		DDRA |= (1U<<port);
	else
		DDRA &= ~(1U<<port);	
}

void ioflip(unsigned char port)
{
	unsigned char state = DDRA & (1U << port);
	
	if (state)
		DDRA &= ~(1U<<port);
	else
		DDRA |= (1U<<port);
}
