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


static void pwm_Init()
{
	DDRB = (1<<PB3) | (1<<PB4);
	TCCR0A = (1 << WGM01) | (1 << WGM00) | (1<<COM0A1);
	TCCR0B = (1<<CS00) /*| (1 << CS01)*/;
	OCR0A = 128;
}

static void adc_Init()
{
	ADMUX = (1<<REFS0);
	ADCSRA = (1<<ADEN)|(1<<ADPS2)|(1<<ADPS1)|(1<<ADPS0);
}
// read adc value
uint16_t adc_read(uint8_t ch)
{
    // select the corresponding channel 0~7
    // ANDing with '7' will always keep the value
    // of 'ch' between 0 and 7
    ch &= 0b00000111;  // AND operation with 7
    ADMUX |= ch;

    // start single conversion
    // write '1' to ADSC
    ADCSRA |= (1<<ADSC);

    // wait for conversion to complete
    // ADSC becomes '0' again
    // till then, run loop continuously
    while(ADCSRA & (1<<ADSC));

    return (ADC);
}

static void hardware_init()
{	
	PORTA = 0;
	config_Init();
	USART_Init();
	RTC_Init();
	pwm_Init();
	adc_Init();
}


int main(void)
{
	cli();
	hardware_init();
	sei();
	uint16_t adc_val;
	uint16_t tick;
	char hz = 0;
	char scale = 0;
	char translation = 128;
	float x;
	
	USART_Send(0, "hello world\n", 12);
	while (1)
	{
		for(tick = 0; tick < 1000; tick++)
		{
			hz = adc_read(0) >> 2;
			scale = adc_read(1) >> 3;
			translation = adc_read(2) >> 2;
			x = M_PI/(500.0/hz);
			OCR0A = (char)round(scale*sin(tick*x) + translation);
			_delay_ms(1);
		}
	}	
	// Never reached.
	return(0);
}

