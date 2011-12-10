#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "config.h"
#include "9p.h"
#include "9p_pos.h"

uint16_t pos_global = 0;

int16_t pos_read(const struct DirectoryEntry *dp, uint16_t tag, uint64_t * offset, uint32_t * count)
{
	uint8_t reply[6];
	
	*((uint16_t *)(reply+4)) = pos_global;
	
	p9_send_reply(Rread, tag, reply, 6);
	return 0;
}

DirectoryEntry * p9_build_pos_dir(uint8_t parent_qid_index, DirectoryEntry * parent)
{
	DirectoryEntry *dir_pos = (DirectoryEntry *)malloc(3 * sizeof(DirectoryEntry));
	if (!dir_pos)
	{
		printf("Dir build malloc fail\n");
		return 0;
	}
	*dir_pos = (DirectoryEntry){"..", {QTDIR, 0, parent_qid_index}, parent};
	*(dir_pos + 1) = (DirectoryEntry){"0", {QTFILE, 0, p9_register_de(dir_pos+1)}, 0, pos_read, p9_nowrite};
	*(dir_pos + 2) = (DirectoryEntry){0};
	
	return dir_pos;
}