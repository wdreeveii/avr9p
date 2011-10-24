#include <avr/io.h>
#include <avr/interrupt.h>
#include "buffer.h"
#include "usart.h"
#include "config.h"
#include "iocontrol.h"
#include "rtc.h"
#include <string.h>
#include <limits.h>
#include <stdlib.h>
#include <stdio.h>

/* lets take care of the glue code between avrlibc and my USART driver */

static int putchar_for_printf(char c, FILE *stream)
{
	if (c == '\n')
		putchar_for_printf('\r', stream);
		USART_Send(0, &c, 1);
	return 0;	
}

static FILE standard_output = FDEV_SETUP_STREAM(putchar_for_printf, NULL, _FDEV_SETUP_WRITE);

/*
 * Input and output buffers declarations
 */
buffer_t in_buf[2], out_buf[2];

/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
   USART_Init : Initialize USART #1

   Purpose : Init the USART with chosen baudrate, number of
             data bits and stop bit and enable the transmitter
             & reciever, and enable the recieve interrupt.
             
             Also enable the buffers used to store data being
             transmitted and recieved by this usart.

   Input : baudrate

   Output : void
   ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */
void USART_Init1(uint16 baud)
{
	/* Wait latest receive character was pull from UDR */
	while(UCSR0A & (1<<RXC0));

	/* Wait end of transmission */
	while(!(UCSR0A & (1<<UDRE0)));

	/* Disable transmitter and receiver */
	UCSR0B &= ~((1<<RXEN0)|(1<<TXEN0));

	/* Init buffers */
	Buffer_Reset(&in_buf[0]);
	Buffer_Reset(&out_buf[0]);
	
	UCSR0A = (1 << U2X0);
	
	/* Set baud rate */
	UBRR0H = (unsigned char)(baud>>8);
	UBRR0L = (unsigned char)baud;

	/* Enable transmitter and receiver (RXEN and TXEN), enable reception interrupt (RXCIE) */
	UCSR0B = (1<<RXCIE0)|(1<<RXEN0)|(1<<TXEN0);
	
	UCSR0C = (3<<UCSZ00);
	
}

/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
   USART_Init : Initialize USART #2

   Purpose : Init the USART with chosen baudrate, number of
             data bits and stop bit and enable the transmitter
             & reciever, and enable the recieve interrupt.
             
             Also enable the buffers used to store data being
             transmitted and recieved by this usart.

   Input : baudrate

   Output : void
   ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */
void USART_Init2(uint16 baud)
{
	/* Wait latest receive character was pull from UDR */
	while(UCSR1A & (1<<RXC1));

	/* Wait end of transmission */
	while(!(UCSR1A & (1<<UDRE1)));

	/* Disable transmitter and receiver */
	UCSR1B &= ~((1<<RXEN1)|(1<<TXEN1));

	/* Init buffers */
	Buffer_Reset(&in_buf[1]);
	Buffer_Reset(&out_buf[1]);

	UCSR1A = (1 << U2X1);

	/* Set baud rate */
	UBRR1H = (unsigned char)(baud>>8);
	UBRR1L = (unsigned char)baud;
	
	/* Enable transmitter and receiver (RXEN and TXEN), enable reception interrupt (RXCIE) */
	UCSR1B = (1<<RXCIE1)|(1<<RXEN1)|(1<<TXEN1);

	UCSR1C = (3<<UCSZ10);
}
/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
   USART_Init : Initialize USART

   Purpose : Init the USART with baudrate stored in config eeprom,
             If no baudrate is found, use a default value of 42 aka 57600baud.

   Input : void

   Output : void
   ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */
void USART_Init(void)
{
	uint16 baud = 42;
	uint16 config_baud = 0;
	// The following lines are helpful when creating a config eeprom from scratch
	//config_set_baud(0, 42);
	//config_set_baud(1, 42);
	if (config_baud = config_get_baud(0)) baud = config_baud;
	USART_Init1(baud);
	// setup stdout so printf works
	stdout = &standard_output;
	
	baud = 42;
	if (config_baud = config_get_baud(1)) baud = config_baud;
	USART_Init2(baud);
}

/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
   Usart_Send : Send a frame

   Purpose : Copy data to output buffer and enable
             UDR empty interrupt

   Input : p_data, pointer on data frame to send
           length, length of frame

   Output : void
   ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */
void USART_Send(char port, char *p_data, unsigned short length)
{
	unsigned short i = 0;
	/* Wait end of transmission -- not sure this is needed*/
	/*if (port == 0)
		while(!(UCSR0A & (1<<UDRE0)));
	else
		while(!(UCSR1A & (1<<UDRE1)));*/
		
	cli();
	// disable interrupts while copying data into the send buffer
	
	/* Copy data into the buffer */
	for(i=0; i<length; i++)
		Buffer_Push(&out_buf[port], *(p_data+i));
		
	sei();
	
	/* Enable UDR Empty interrupt */
	if (port == 0)
		UCSR0B |= (1<<UDRIE0);
	else
		UCSR1B |= (1<<UDRIE1);
}

/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
   Interrupt Routine : UDR Empty

   Purpose : Try to take a character from output buffer. If
             buffer is empty (character == -1), disable this
             interrupt. Else, put char in UDR.

   Input : void

   Output : void
   ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */
ISR(USART0_UDRE_vect)
{
	char c;	
	if(Buffer_Pull(&out_buf[0], (unsigned char *)&c) == -1)					// If buffer empty
		UCSR0B &= ~(1<<UDRIE0);	// Stop UDR empty interrupt : Tx End
	else
		UDR0 = c;											// Else, put c in UDR
}

ISR(USART1_UDRE_vect)
{
	char c;	
	if(Buffer_Pull(&out_buf[1], (unsigned char *)&c) == -1)					// If buffer empty
		UCSR1B &= ~(1<<UDRIE1);	// Stop UDR empty interrupt : Tx End
	else
		UDR1 = c;											// Else, put c in UDR
}

void cmd_switch_output(buffer_t *buf, unsigned char data_size)
{
	unsigned char port;
	unsigned char state;
	
	Buffer_Pull(buf, &port);
	Buffer_Pull(buf, &state);
	iocontrol(port, state);
	DSEND(0, "End Switch Output\n");
}

#define MAX_INT_STR_LEN 11
void cmd_set_time(buffer_t *buf, unsigned char data_size)
{
	char unixtimestr[MAX_INT_STR_LEN];
	unsigned char unixtimestrlen = 0;
	time_t unixtime;
	
	for (; unixtimestrlen < data_size; unixtimestrlen++)
	{
		Buffer_Pull(buf, unixtimestr + unixtimestrlen);
	}
	unixtimestr[unixtimestrlen] = 0;
	unixtime = (time_t)strtoul(unixtimestr, NULL, 10);
	set_time(unixtime);
}

void cmd_set_timer_event(buffer_t *buf, unsigned char data_size)
{
	char tmp[data_size];
	char index;
	for (index = 0; index < data_size; index++)
	{
		Buffer_Pull(buf, tmp + index);
	}
	mucron_save_event((struct s_mucron *)tmp);
}

void cmd_delete_timer_event(buffer_t *buf, unsigned char data_size)
{
	char tmp[data_size];
	char index;
	for (index = 0; index < data_size; index++)
	{
		Buffer_Pull(buf, tmp+index);
	}
	if (data_size)
	{
		mucron_delete_event(tmp[0]);
	}
}

#define HEADER_SIZE 2
#define MAX_PACKET_SIZE 62

#define CMD_SWITCH_OUTPUT 1
#define CMD_SET_TIME 2
#define CMD_GET_TIME 3
#define CMD_SET_TIMER_EVENT 4
#define CMD_GET_TIMER_EVENTS 5
#define CMD_DELETE_TIMER_EVENT 6
#define CMD_PING 7

void process_packet(buffer_t *buf)
{
	unsigned char packet_size;
	unsigned char checksum;
	unsigned char packet_type;
	
	Buffer_Pull(buf, &packet_size);
	Buffer_Pull(buf, &checksum);
	Buffer_Pull(buf, &packet_type);
	
	switch (packet_type)
	{
		case CMD_SWITCH_OUTPUT:
			cmd_switch_output(buf, packet_size - 1);
			break;
		case CMD_SET_TIME:
			cmd_set_time(buf, packet_size - 1);
			break;
		case CMD_GET_TIME:
			break;
		case CMD_SET_TIMER_EVENT:
			cmd_set_timer_event(buf, packet_size - 1);
			break;
		case CMD_GET_TIMER_EVENTS:
			mucron_list_events();
			break;
		case CMD_DELETE_TIMER_EVENT:
			cmd_delete_timer_event(buf, packet_size - 1);
			break;
		case CMD_PING:
			DSEND(0, "PONG\n");
			break;
	}
}
/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
   Interrupt Routine : Receive complete

   Purpose : Check frame error and parity error. If they are at
             least one error, read character from UDR and put
             it in garbage. Else, push it into input buffer.

   Input : void

   Output : void
   ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */
ISR(USART0_RX_vect)
{
	char garbage;
	if((UCSR0A & (1<<FE0))||(UCSR0A & (1<<UPE0)))	// If frame error or parity error
		garbage = UDR0;							// UDR -> Garbage
	else
		Buffer_Push(&in_buf[0], UDR0);				// else, send received char into input buffer
	
	//do we know the packet size?
	if (in_buf[0].count > 1)
	{
		// have all the bytes of the packet been recieved
		if (*(in_buf[0].p_out) + HEADER_SIZE == in_buf[0].count)
		{
			process_packet(&in_buf[0]);
			Buffer_Reset(&in_buf[0]);
		}
	}
}

ISR(USART1_RX_vect)
{
	char garbage;

	if((UCSR1A & (1<<FE1))||(UCSR1A & (1<<UPE1)))	// If frame error or parity error
		garbage = UDR1;							// UDR -> Garbage
	else
		Buffer_Push(&in_buf[1], UDR1);				// else, send received char into input buffer
}

ISR(BADISR_vect)
{
   iocontrol(2,1);
}
