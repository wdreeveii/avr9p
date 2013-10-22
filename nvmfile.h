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
//  nvmfile.h
//

#ifndef _NVMFILE_H
#define _NVMFILE_H

#include <avr/pgmspace.h>
#include "9p_pgm.h"
#include "config.h"
#include "vm.h"

typedef struct {
  uint8_t super;
  uint8_t fields;
} PACKED nvm_class_hdr_t;

typedef struct {
  uint16_t code_index;
  uint16_t id;           // class and method id
  uint8_t flags;
  uint8_t args;
  uint8_t max_locals;
  uint8_t max_stack;
} PACKED nvm_method_hdr_t;

typedef struct {
  uint32_t magic_feature;    // old 32 bit magic is replaced by 8 bit magic and 24 feauture bits
  uint8_t version;
  uint8_t methods;          // number of methods in this file
  uint16_t main;             // index of main method
  uint16_t constant_offset;
  uint16_t string_offset;
  uint16_t method_offset;
  uint8_t static_fields;
  nvm_class_hdr_t class_hdr[];
} PACKED nvm_header_t;

// marker that indicates, that a method is a classes init method
#define FLAG_CLINIT 1

extern uint8_t nvmfile_constant_count;

void   nvmfile_store(uint16_t index, uint8_t *buffer, uint16_t size);

bool_t nvmfile_init(void);
void   nvmfile_call_main(void);
void   *nvmfile_get_addr(uint16_t ref);
uint8_t  nvmfile_get_class_fields(uint8_t index);
uint8_t  nvmfile_get_static_fields(void);
uint32_t  nvmfile_get_constant(uint8_t index);
PGM_VOID_P nvmfile_get_base(void);
uint8_t  nvmfile_get_method_by_class_and_id(uint8_t class, uint8_t id);
nvm_method_hdr_t *nvmfile_get_method_hdr(uint16_t index);

void   nvmfile_read(void *dst, const void *src, uint16_t len);
uint8_t  nvmfile_read08(const void *addr);
uint16_t  nvmfile_read16(const void *addr);
uint32_t  nvmfile_read32(const void *addr);
void   nvmfile_write08(void *addr, uint8_t data);

#define NVMFILE_SET(a)     (void*)(((ptr_t)a) | NVMFILE_FLAG)
#define NVMFILE_ISSET(a)   (((ptr_t)a) & NVMFILE_FLAG)
#define NVMFILE_ADDR(a)    (void*)(((ptr_t)a) & ~NVMFILE_FLAG)

#define NVMFILE_FLAG     0x8000

void nvmfile_write_initialize(void);
void nvmfile_write_finalize(void);

void nvmfile_load(const char *filename, bool_t quiet);

#endif // NVMFILE_H
