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
//  main() for NanoVM runtime
//

#include <stdio.h>

#include "types.h"
#include "config.h"
#include "debug.h"

#include "loader.h"
#include "../usart.h"
#include "nvmfile.h"
#include "vm.h"

// hooks for init routines

#include "native_impl.h"


int main(int argc, char **argv) {

//loader_receive();

  nvmfile_init();

  vm_init();

  nvmfile_call_main();
  for(;;);  // reset wdt if in use

  return 0;
}
