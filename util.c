
#include "config.h"
#include <util/delay.h>
#include "util.h"

void delay_1s(void)
{
  uint8_t i;

  for (i = 0; i < 100; i++)
    _delay_ms(10);
}