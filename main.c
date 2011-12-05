// Firmware for fdio controller V1
#include "config.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include <math.h>

#include "usart.h"
#include "util.h"
#include "iocontrol.h"
#include "rtc.h"
#include "softtimer.h"
#include "9p.h"

static void hardware_init()
{	
	PORTA = 0;
	soft_timer_init();
	config_Init();
	USART_Init();
	RTC_Init();
}

uint8_t testtimerslot;
void test()
{
	DSEND(0, "whats up\n");
	reset_timer(time() + 5, &test, testtimerslot);	
}

int main(void)
{
	uint8_t mcusr;
	cli();
	hardware_init();
	p9_init();
	mcusr = MCUSR;
	MCUSR = 0;
	sei();
	printf("mcusr %x\n", mcusr);
	USART_Send(0, (uint8_t*)"hello world\n", 12);
	
	testtimerslot = set_timer(time() + 5, &test);
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

