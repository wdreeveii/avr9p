// Firmware for fdio controller V1
#include "config.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include <util/atomic.h>
#include <math.h>

#include "usart.h"
#include "util.h"
#include "iocontrol.h"
#include "spi.h"
#include "rtc.h"
#include "softtimer.h"
#include "9p.h"
#include "9p_pgm.h"

static void hardware_init()
{
	DDRA = 0;
	DDRB = 0;
	DDRC = 0xC0;
	DDRD = 0;
	PORTA = 0;
	PORTB = 0;
	PORTC = 0xC0;
	PORTD = 0;
	io_init();
	spi_init();
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
	uint8_t retval;
	cli();
	hardware_init();
	p9_init();
	mcusr = MCUSR;
	MCUSR = 0;
	sei();
	printf("Arctic Precision Horticulture\n");
	printf("Model: APC5 Firmware: 5.15\n");
	printf("mcusr %x\n", mcusr);
	
	
	testtimerslot = set_timer(time() + 5, &test);
	while (1)
	{
		  /*retval = nvmfile_init();
		  printf("retval: %d\n", retval);
		  if (retval)
		  {
		  	  vm_init();
		  	  nvmfile_call_main();
		  }
		_delay_ms(1);*/
		//write_spi_mem();
	}	
	// Never reached.
	return(0);
}

ISR(BADISR_vect)
{
   DDRC |= (1U<<5);
}

