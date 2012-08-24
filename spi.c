#include <avr/io.h>
#include <stdio.h>
#include "config.h"

void spi_init()
{
	DDRB |= 0xB0;
	SPCR = (1 << SPE) | (1 << MSTR) | (1 << SPR0);
	
}

uint8_t spi_tx(uint8_t data)
{	/* Start transmission */
	SPDR = data;
	/* Wait for transmission complete */
	while(!(SPSR & (1<<SPIF)));
	return SPDR;
}

#define CS 6
void write_spi_mem()
{
	/* set chip select low */
	PORTC &= ~(1<<CS);
	//printf("%i\n", spi_tx(0x06));
	printf("%i\n", spi_tx(0x05));
	printf("%i\n", spi_tx(0x00));
	PORTC |= (1<<CS);
}