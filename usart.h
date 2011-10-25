
#ifndef _USART_H
#define _USART_H

void USART_Init(void);
void USART_Send(char port, char *p_out, unsigned short length);

#define DSEND(p, s) USART_Send(p, s, strlen(s))

#endif