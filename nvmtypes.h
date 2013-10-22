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
//  vmtypes.h
//

#ifndef _NVMTYPES_H
#define _NVMTYPES_H

// THE basic types
typedef uint32_t nvm_word_t;
typedef uint32_t nvm_stack_t;
typedef uint32_t nvm_ref_t;
typedef int32_t nvm_int_t;
typedef uint32_t nvm_uint_t; // needed for >>> operator
typedef int16_t nvm_short_t;
typedef int8_t nvm_byte_t;
typedef float nvm_float_t;
// masks and bits of the basic type
#define NVM_IMMEDIATE_MASK  ((nvm_ref_t)0x80000000L)
#define NVM_TYPE_MASK       ((nvm_ref_t)0xc0000000L)
#define NVM_TYPE_HEAP       ((nvm_ref_t)0x80000000L)
#define NVM_TYPE_CONST      ((nvm_ref_t)0xc0000000L)
typedef union
{
  nvm_byte_t  b[4];
  nvm_short_t s[2];
  nvm_int_t   i[1];
  nvm_float_t f[1];
} nvm_union_t;

#endif // _NVMTYPES_H
