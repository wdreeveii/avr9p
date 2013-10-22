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
#include "config.h"
#include <avr/io.h>

#include <util/delay.h>

#include "types.h"
#include "debug.h"
#include "error.h"

char *error_msg[] = {
  // unix message              avr error code
  "HEAP: illegal chunk size",        // A
  "HEAP: corrupted",                 // B
  "HEAP: out of memory",             // C
  "HEAP: chunk does not exist",      // D
  "HEAP: out of stack memory",       // E
  "HEAP: stack underrun",            // F
  "HEAP: out of identifiers",        // G
  "ARRAY: illegal type",             // H
  "NATIVE: unknown method",          // I
  "NATIVE: unknown class",           // J
  "NATIVE: illegal argument",        // K
  "NVMFILE: unsupported features or not a valid nvm file",   // L
  "NVMFILE: wrong nvm file version", // M
  "VM: illegal reference",           // N
  "VM: unsupported opcode",          // O
  "VM: division by zero",            // P
  "VM: stack corrupted",             // Q
};
void error(err_t code) {
  //uart_putc('E');
  //uart_putc('R');
  //uart_putc('R');
  //uart_putc(':');
  //uart_putc('A'+code);
  //uart_putc('\n');

  for(;;) {
    // reset watchdog here if in use

  }
}
