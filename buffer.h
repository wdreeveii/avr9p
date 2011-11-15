
#ifndef _BUFFER_H
#define _BUFFER_H

/*  Struct buffer_t  */
#define BUFFER_SIZE 256

typedef struct{
  unsigned char tab[BUFFER_SIZE];
  unsigned char *p_in, *p_out;
  unsigned char count;
}buffer_t;


/*  Function's prototype  */
void Buffer_Reset(buffer_t *buffer);
char Buffer_Push(buffer_t *buffer, unsigned char byte);
char Buffer_Pull(buffer_t *buffer, unsigned char *byte);

#endif