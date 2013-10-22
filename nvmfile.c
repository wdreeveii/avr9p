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
//  nvmfile.c
//
//  routines to store and access the NanoVM internal nvm file
//  format as generated by NanoVMTool
//

#include "config.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "9p_pgm.h"
#include "error.h"

#include "nvmfile.h"
#include "nvmtypes.h"
#include "nvmfeatures.h"
#include "vm.h"


#include <avr/io.h>
#include <util/atomic.h>
#include <avr/pgmspace.h>

uint8_t nvmfile_constant_count;

PGM_VOID_P pgm_mem = NULL;

PGM_VOID_P nvmfile_get_base(void) {
  return pgm_mem;
}


void nvmfile_read(void *dst, const void *src, uint16_t len)
{
  src = NVMFILE_ADDR(src);  // remove marker (if present)
  memcpy_P(dst, (PGM_P)src, len);
}

uint8_t nvmfile_read08(const void *addr)
{
  uint8_t val;
  addr = NVMFILE_ADDR(addr);  // remove marker (if present)
  memcpy_P((uint8_t*)&val, (PGM_P)addr, sizeof(val));
  return val;
}

uint16_t nvmfile_read16(const void *addr)
{
  uint16_t val;
  addr = NVMFILE_ADDR(addr);  // remove marker (if present)
  memcpy_P((uint8_t*)&val, (PGM_P)addr, sizeof(val));
  return val;
}

uint32_t nvmfile_read32(const void *addr)
{
  uint32_t val;
  addr = NVMFILE_ADDR(addr);  // remove marker (if present)
  memcpy_P((uint8_t*)&val, (PGM_P)addr, sizeof(val));
  return val;
}

void nvmfile_write08(void *addr, uint8_t data)
{

}

void nvmfile_write_initialize()
{

}

void nvmfile_write_finalize()
{

}

void nvmfile_store(uint16_t index, uint8_t *buffer, uint16_t size)
{

}


bool_t nvmfile_init(void)
{
	uint16_t t;
	nvm_header_t * nvm_header;
	PGM_VOID_P targetcopy;
  	
  	ATOMIC_BLOCK(ATOMIC_RESTORESTATE)
	{
			targetcopy = pgm_target;
	}
	if (targetcopy == NULL)
	{
		return FALSE;
	}
	else
	{
		pgm_mem = targetcopy;
	}
	
	nvm_header = (nvm_header_t *)pgm_mem;
	
	uint32_t features = nvmfile_read32(&nvm_header->magic_feature);
	DEBUGF("NVM_MAGIC_FEAUTURE[file] = %lx\n", features);
	DEBUGF("NVM_MAGIC_FEAUTURE[vm] = %lx\n", NVM_MAGIC_FEAUTURE);
	
	if ((features&NVM_MAGIC_FEAUTURE)!=(features|NVMFILE_MAGIC))
	{
		error(ERROR_NVMFILE_MAGIC);
		return FALSE;
	}
	
	if(nvmfile_read08(&nvm_header->version) != NVMFILE_VERSION) {
		error(ERROR_NVMFILE_VERSION);
		return FALSE;
	}
	
	t  = nvmfile_read16(&nvm_header->string_offset);
	t -= nvmfile_read16(&nvm_header->constant_offset);
	nvmfile_constant_count = t/4;
	
	return TRUE;
}

nvm_method_hdr_t *nvmfile_get_method_hdr(uint16_t index)
{
  // get pointer to method header
  nvm_header_t * nvm_header = (nvm_header_t *)pgm_mem;
  nvm_method_hdr_t *hdrs =
    ((nvm_method_hdr_t*)(pgm_mem +
	 nvmfile_read16(&nvm_header->method_offset)))+index;

  return(hdrs);
}

uint32_t nvmfile_get_constant(uint8_t index)
{
  nvm_ref_t res;

  if (index<nvmfile_constant_count)
  {
    nvm_header_t * nvm_header = (nvm_header_t *)pgm_mem;
    uint16_t addr = nvmfile_read16(&nvm_header->constant_offset);
    uint32_t result = nvmfile_read32(pgm_mem+addr+4*index);
    DEBUGF("  constant = 0x%08lx\n", result);
    return result;
  }
  // it's a string!

  DEBUGF("  constant string index = %i\n", index);
  res = NVM_TYPE_CONST | (index-nvmfile_constant_count);
  return res;
}

void nvmfile_call_main(void)
{
  uint8_t i;

  nvm_header_t * nvm_header = (nvm_header_t *)pgm_mem;
  for(i=0;i<nvmfile_read08(&nvm_header->methods);i++) {
    // is this a clinit method?
    if(nvmfile_read08(&nvmfile_get_method_hdr(i)->flags) & FLAG_CLINIT) {
      DEBUGF("calling clinit %d\n", i);
      vm_run(i);
    }
  }

  // determine method description address and code
  vm_run(nvmfile_read16(&nvm_header->main));
}

void *nvmfile_get_addr(uint16_t ref)
{
  // get pointer to string
  nvm_header_t * nvm_header = (nvm_header_t *)pgm_mem;
  uint16_t *refs =
    (uint16_t*)(pgm_mem +
	     nvmfile_read16(&nvm_header->string_offset));

  return((uint8_t*)refs + nvmfile_read16(refs+ref));
}

uint8_t nvmfile_get_class_fields(uint8_t index)
{
  nvm_header_t * nvm_header = (nvm_header_t *)pgm_mem;
  return nvmfile_read08(&nvm_header->class_hdr[index].fields);
}

uint8_t nvmfile_get_static_fields(void)
{
  nvm_header_t * nvm_header = (nvm_header_t *)pgm_mem;
  return nvmfile_read08(&nvm_header->static_fields);
}

uint8_t nvmfile_get_method_by_fixed_class_and_id(uint8_t class, uint8_t id) {
  uint8_t i;
  nvm_method_hdr_t mhdr, *mhdr_ptr;

  DEBUGF("Searching for class "DBG8", method "DBG8"\n", class, id);

  nvm_header_t * nvm_header = (nvm_header_t *)pgm_mem;
  for(i=0;i<nvmfile_read08(&nvm_header->methods);i++) {
    DEBUGF("Method %d ", i);
    // load new method header into ram
    mhdr_ptr = nvmfile_get_method_hdr(i);
    nvmfile_read(&mhdr, mhdr_ptr, sizeof(nvm_method_hdr_t));
    DEBUGF("id = #"DBG16"\n", mhdr.id);

    if(((mhdr.id >> 8) == class) && ((mhdr.id & 0xff) == id)) {
      DEBUGF("Match!\n");
      return i;
    }
  }

  DEBUGF("No matching method in this class\n");
  return 0xff;
}

uint8_t nvmfile_get_method_by_class_and_id(uint8_t class, uint8_t id) {
  uint8_t mref;

  nvm_header_t * nvm_header = (nvm_header_t *)pgm_mem;
  for(;;) {
    if((mref = nvmfile_get_method_by_fixed_class_and_id(class, id)) != 0xff)
      return mref;

    DEBUGF("Getting super class of %d ", class);
    class = nvmfile_read08(&nvm_header->class_hdr[class].super);
    DEBUGF("-> %d\n", class);
  }

  return 0;
}