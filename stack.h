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
//  stack.h
//

#ifndef _STACK_H
#define _STACK_H

#include "vm.h"

void stack_init(uint8_t static_fields);

void stack_save_sp(void);
void stack_verify_sp(void);

// various stack operations
void stack_push(nvm_stack_t val);
nvm_stack_t stack_pop(void);
nvm_int_t stack_pop_int(void);
nvm_stack_t stack_peek(uint8_t index);
nvm_int_t stack_peek_int(uint8_t index);
void * stack_pop_addr(void);
void * stack_peek_addr(uint8_t index);

nvm_float_t stack_pop_float(void);
nvm_float_t stack_peek_float(uint8_t index);


nvm_stack_t *stack_get_sp(void);
void stack_add_sp(int8_t offset);

nvm_stack_t stack_get_static(uint16_t index);
void stack_set_static(uint16_t index, nvm_stack_t value);

void stack_save_base(void);
bool_t stack_is_empty(void);

uint16_t stack_get_depth(void);

#ifdef NVM_USE_HEAP_IDMAP
void stack_mark_heap_root_ids(void);
#else
bool_t stack_heap_id_in_use(heap_id_t id);
#endif

#endif // STACK_H
