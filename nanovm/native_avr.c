//
//  NanoVM, a tiny java VM for the Atmel AVR family
//  Copyright (C) 2005 by Till Harbaum <Till@Harbaum.org>
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation; either version 2 of the License, or
//  (at your option) any later version.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program; if not, write to the Free Software
//  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
//

//
//  native_avr.c
//

#include "types.h"
#include "debug.h"
#include "config.h"
#include "error.h"

#include "vm.h"
#include "native.h"
#include "native_avr.h"
#include "stack.h"
#include "../usart.h"

#include <avr/io.h>
#include <avr/interrupt.h>

volatile u08_t *ports[] = { &PORTA, &PORTB, &PORTC, &PORTD };
volatile u08_t *ddrs[]  = { &DDRA,  &DDRB,  &DDRC,  &DDRD  };
volatile u08_t *pins[]  = { &PINA,  &PINB,  &PINC,  &PIND  };

volatile static nvm_int_t ticks;

ISR(TIMER1_COMPA_vect ) {
  TCNT1 = 0;
  ticks++;
}

void native_init(void) {
  // init timer
  TCCR1B = _BV(CS11);           // clk/8
  OCR1A = (u16_t)(CLOCK/800u);  // 100 Hz is default
  TIMSK1 |= _BV(OCIE1A);         // interrupt on compare
}

// the AVR class
void native_avr_avr_invoke(u08_t mref) {
  if(mref == NATIVE_METHOD_GETCLOCK) {
    stack_push(CLOCK/1000);
  } else
    error(ERROR_NATIVE_UNKNOWN_METHOD);
}

// the port class
void native_avr_port_invoke(u08_t mref) {
  if(mref == NATIVE_METHOD_SETINPUT) {
    u08_t bit  = stack_pop();
    u08_t port = stack_pop();
    DEBUGF("native setinput %bd/%bd\n", port, bit);
    *ddrs[port] &= ~_BV(bit);
  } else if(mref == NATIVE_METHOD_SETOUTPUT) {
    u08_t bit  = stack_pop();
    u08_t port = stack_pop();
    DEBUGF("native setoutput %bd/%bd\n", port, bit);
    *ddrs[port] |= _BV(bit);
  } else if(mref == NATIVE_METHOD_SETBIT) {
    u08_t bit  = stack_pop();
    u08_t port = stack_pop();
    DEBUGF("native setbit %bd/%bd\n", port, bit);
    *ports[port] |= _BV(bit);
  } else if(mref == NATIVE_METHOD_CLRBIT) {
    u08_t bit  = stack_pop();
    u08_t port = stack_pop();
    DEBUGF("native clrbit %bd/%bd\n", port, bit);
    *ports[port] &= ~_BV(bit);
  } else
    error(ERROR_NATIVE_UNKNOWN_METHOD);
}

// the timer class, based on AVR 16 bit timer 1
void native_avr_timer_invoke(u08_t mref) {
  if(mref == NATIVE_METHOD_SETSPEED) {
    OCR1A = stack_pop_int();  // set reload value
  } else if(mref == NATIVE_METHOD_GET) {
    stack_push(ticks);
  } else if(mref == NATIVE_METHOD_TWAIT) {
    nvm_int_t wait = stack_pop_int();
    ticks = 0;
    while(ticks < wait);      // reset watchdog here if enabled
  } else if(mref == NATIVE_METHOD_SETPRESCALER) {
    TCCR1B = stack_pop_int();
  } else
    error(ERROR_NATIVE_UNKNOWN_METHOD);
}

// the Adc class
void native_avr_adc_invoke(u08_t mref) {
}


void native_avr_pwm_invoke(u08_t mref) {
}


