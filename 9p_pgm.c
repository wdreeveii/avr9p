#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <util/atomic.h>

#include "9p.h"


void pgm_do_write( void ) __attribute__ ((section (".boot")));

void pgm_do_write( void )
{
	
} 

int16_t pgm_load(const struct DirectoryEntry *dp, uint64_t *offset, uint32_t *count, uint8_t *indata) __attribute__ ((section (".boot")));

int16_t pgm_load(const struct DirectoryEntry *dp, uint64_t *offset, uint32_t *count, uint8_t *indata)
{
	uint8_t datacpy[*count + 1];
	datacpy[*count] = 0;
	uint16_t index;
	memcpy(datacpy, indata, *count);
	if (sscanf((char *)datacpy, "%u", &index) == 1)
	{
	}
	else return -1;
	return *count;
}

int16_t pgm_erase(const struct DirectoryEntry *dp, uint64_t *offset, uint32_t *count, uint8_t *indata)
{
	uint8_t datacpy[*count + 1];
	datacpy[*count] = 0;
	uint16_t index;
	memcpy(datacpy, indata, *count);
	if (sscanf((char *)datacpy, "%u", &index) == 1)
	{
	}
	else return -1;
	return *count;
}

int16_t pgm_list(uint8_t oc, const struct DirectoryEntry *dp, uint16_t tag, uint64_t * offset, uint32_t * count)
{
	uint8_t reply[20];
	uint8_t len = 0;
	
	if (*(offset) == 0)
		*((uint32_t *)reply) = len = sprintf((char *)(reply + 4), "%lu\n", (uint32_t)0);

	p9_send_reply(oc, Rread, tag, reply, len + 4);
	return 0;
}

DirectoryEntry dir_pgm[5];
DirectoryEntry * p9_build_pgm_dir(uint8_t parent_qid, DirectoryEntry * parent)
{
	dir_pgm[0] = (DirectoryEntry){"..", {QTDIR, 0, parent_qid}, parent};
	dir_pgm[1] = (DirectoryEntry){"load", {QTFILE, 0, p9_register_de(dir_pgm+1)}, 0, p9_noread, pgm_load};
	dir_pgm[2] = (DirectoryEntry){"erase",{QTFILE, 0, p9_register_de(dir_pgm+2)}, 0, p9_noread, pgm_erase};
	dir_pgm[3] = (DirectoryEntry){"list", {QTFILE, 0, p9_register_de(dir_pgm+3)}, 0, pgm_list, p9_nowrite};
	dir_pgm[4] = (DirectoryEntry){0};
	
	return dir_pgm;
}