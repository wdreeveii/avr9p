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
	
	USART_Send(0, "hello world\n", 12);
	USART_Send(1, "hello world\n", 12);
	while (1)
	{
		DSEND(1, "BLAHHHHHHH TESTING\r\n");
	}	
	// Never reached.
	return(0);
}

ISR(BADISR_vect)
{
   iocontrol(2,1);
}

