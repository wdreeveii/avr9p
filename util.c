
#include "config.h"
#include <util/delay.h>
#include "util.h"

void delay_1s(void)
{
  uint8_t i;

  for (i = 0; i < 100; i++)
    _delay_ms(10);
}

/*unsigned char get_byte( void )
{
    unsigned char n=0;
    char ch;

    while(1)
    {
        ch =getchar();
        if (ch == 0x0a) break;
        n *=10;
        n += ch - '0';
    }
    return n;
}*/
