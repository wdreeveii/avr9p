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
//  stack.c
//

#include "types.h"
#include "debug.h"
#include "config.h"
#include "error.h"

#include "vm.h"
#include "heap.h"
#include "stack.h"

// the stack
static nvm_stack_t *stack;     // the pysical base of the whole stack (incl. statics)
static nvm_stack_t *sp;        // the current stack pointer
static nvm_stack_t *stackbase; // the base of the runtime stack (excl. statics)

#ifdef NVM_USE_STACK_CHECK
nvm_stack_t *sp_saved = NULL;

// save current stack pointer
void stack_save_sp(void) {
  if(sp_saved != NULL) {
    DEBUGF("sp_saved already in use\n");
    error(ERROR_VM_STACK_CORRUPTED);
  }

  sp_saved = sp;  // save stack pointer
}

// check if current stack pointer equals saved one
void stack_verify_sp(void) {
  if(sp != sp_saved) {
    DEBUGF("%d bytes on stack\n", (u08_t*)sp-(u08_t*)sp_saved);
    error(ERROR_VM_STACK_CORRUPTED);
  }

  sp_saved = NULL;
}
#endif

void stack_init(u08_t static_fields) {

  // the stack is generated by stealing from the heap. This
  // is possible since the class file tells us how many stack
  // elements the call to a method requires
  stack = (nvm_stack_t*)heap_get_base();
  sp = stack-1;

  // steal one item for mains args and the space required for 
  // the static fields
  heap_steal((1+static_fields)*sizeof(nvm_stack_t)); 

  // increase stack pointer behind static fields
  sp += static_fields;
}

// push an item onto the vms stack
void stack_push(nvm_stack_t val) {
  *(++sp) = val;
}

// pop an item from the vms stack
nvm_stack_t stack_pop(void) {
  return *(sp--);
}

// pop an item from the vm stack
nvm_int_t stack_pop_int(void) {
  return nvm_stack2int(*(sp--));
}

// peek into the nvm stack
nvm_stack_t stack_peek(u08_t index) {
  return sp[-index];
}

// peek into the stack and expand 15->16 bits
nvm_int_t stack_peek_int(u08_t index) {
  return nvm_stack2int(sp[-index]);
}

// pop address from stack
void *stack_pop_addr(void) {
  return vm_get_addr(*(sp--));
}

// peek into the nvm stack for address
void *stack_peek_addr(u08_t index) {
  return vm_get_addr(sp[-index]);
}

nvm_float_t stack_pop_float(void)
{
  return nvm_stack2float(*(sp--));
}

nvm_float_t stack_peek_float(u08_t index)
{
  return nvm_stack2float(sp[-index]);
}

// the following two routines are used during method invocation and return
// they are used to make space for local variables on the stack
// and to determine the current stack address to be used for the local
// variables
void stack_add_sp(s08_t offset) {
  sp += offset;
}

nvm_stack_t * stack_get_sp(void) {
  return sp;
}

// static variables are allocated at vm startup on the stack. the following
// two routines provide access to these variables
nvm_stack_t stack_get_static(u16_t index) {
  return stack[index];
}

void stack_set_static(u16_t index, nvm_stack_t value) {
  stack[index] = value;
}

// our way to determine if the java application is being
// finished is to check if the return instruction would cause the
// stack to underflow. this happens, since the main method ends
// with a return statement like any other method. the following
// two methods are used to save the state of the empty stack (there
// may be already the static variables be on the stack) and to check
// whether the stack is filled with the static variables only (it is empty)
void stack_save_base(void) {
  stackbase = stack_get_sp();
}

bool_t stack_is_empty(void) {
  DEBUGF("stack base depth: %d\n", sp-stackbase);
  return(sp == stackbase);
}

#ifdef DEBUG
u16_t stack_get_depth(void) {
  return sp-stack;
}
#endif

#ifdef NVM_USE_HEAP_IDMAP
void stack_mark_heap_root_ids(void) {
  u16_t i;

  DEBUGF("stack_mark_heap_root_ids()\n");
  // since the locals are physically part of the stack we only need
  // to search the stack
  for(i=0;i<sp-stack+1;i++) {

    // we are searching for heap objects only
    if((stack[i] & NVM_TYPE_MASK) == NVM_TYPE_HEAP)
      heap_mark_id(stack[i] & ~NVM_TYPE_MASK);
  }
}
#else
bool_t stack_heap_id_in_use(heap_id_t id) {
  // we are searching for heap objects only
  u16_t i;
  nvm_ref_t id16 = id | NVM_TYPE_HEAP;

  // since the locals are physically part of the stack we only need
  // to search the stack
  for(i=0;i<sp-stack+1;i++) {
//    DEBUGF("Stack %d == "DBG16"\n", i, stack[i]);
    if(stack[i] == id16) return TRUE;
  }

  return FALSE;
}
#endif
