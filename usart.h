
#ifndef _USART_H
#define _USART_H

#include "buffer.h"
enum connection_type {
	REPL = 1,
	PLAN9FS = 2,
	DATASTREAM = 3
};
typedef void (*conn_handler)(uint8_t port, buffer_t * buf);
void USART_Init(void);
void USART_Init0(uint16_t baud);
void USART_Init1(uint16_t baud);
void USART_Send(uint8_t port, uint8_t *p_out, uint16_t length);

#define DSEND(p, s) USART_Send(p, (uint8_t*)s, strlen(s))

#endif