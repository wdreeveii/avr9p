
#ifndef _9P_PGM_H
#define _9P_PGM_H

#include "9p.h"
#include <avr/pgmspace.h>
#define CODESIZE 16384
#define HEAPSIZE 768

extern uint8_t pgm_mem_a[CODESIZE];
extern uint8_t pgm_mem_b[CODESIZE];

extern PGM_VOID_P pgm_target;
DirectoryEntry * p9_build_pgm_dir(uint8_t parent_qid, DirectoryEntry * parent);

#endif