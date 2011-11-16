// Firmware for fdio controller V1
#include "config.h"

#include <stdlib.h>
#include <string.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include <math.h>

#include "usart.h"
#include "util.h"
#include "iocontrol.h"
#include "rtc.h"

static void hardware_init()
{	
	PORTA = 0;
	config_Init();
	USART_Init();
	RTC_Init();
}


int main(void)
{
	cli();
	hardware_init();
	sei();
	
	while (1)
	{
		
	}	
	// Never reached.
	return(0);
}

ISR(BADISR_vect)
{
   iocontrol(2,1);
}

