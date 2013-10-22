//
//  NanoVM, a tiny java VM for the Atmel AVR family
//  Copyright (C) 2005-2006 by Till Harbaum <Till@Harbaum.org>,
//                             Oliver Schulz <whisp@users.sf.net>
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
// The NanoVM boot-loader.
//


#include "loader.h"

#include "types.h"
#include "config.h"
#include "debug.h"
#include "../usart.h"
#include "nvmfile.h"
#include "nvmcomm1.h"
#include <avr/io.h>




void loader_receive(void) {
  u08_t i, block = 1, sum = 0, len;
  u08_t *addr = nvmfile_get_base();
  loader_t loader;

  // tell sender that we accept data
  //uart_write_byte(ASCII_NAK);

  // wait for data with timeout
  //for(sum=0;sum<250 && !uart_available();sum++)
    //delay(MILLISEC(4));

  if(/*uart_available()*/ 1) {
    nvmfile_write_initialize();
    do {
      // try to receive a full data block
      //len = uart_get_block((u08_t*)&loader, sizeof(loader));

      if(len == sizeof(loader)) {
	for(sum=0,i=0;i<LOADER_BLOCK_SIZE;i++)
	  sum += loader.data[i];

	// check for valid loader block
	if((loader.soh == ASCII_SOH) &&
	   (loader.block == block) &&
	   (loader.nblock == (0xff & ~block)) &&
	   (loader.sum == sum)) {

	  // write data to eeprom
	  for(i=0;i<LOADER_BLOCK_SIZE;i++)
	    nvmfile_write08(addr++, loader.data[i]);

	  // send ack and increase block counter
	  //uart_write_byte(ASCII_ACK);
	  block++;
	} //else
	  //uart_write_byte(ASCII_NAK);
      } else {
	if(!len)
	{
	  //uart_write_byte(ASCII_ACK);
	} else {
	  // not a full packet and not the eof marker -> nak
	 // if(loader.soh != ASCII_EOF)
	    //uart_write_byte(ASCII_NAK);
	}
      }
    } while((len < 1) || (loader.soh != ASCII_EOF));
    nvmfile_write_finalize();
    //uart_write_byte(ASCII_ACK);

    for(;;);  // reset watchdog here if enabled
  }
}

