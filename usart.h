
#ifndef _USART_H
#define _USART_H

void USART_Init(void);
void USART_Send(uint8_t port, uint8_t *p_out, uint16_t length);

#define DSEND(p, s) USART_Send(p, (uint8_t*)s, strlen(s))

#endif