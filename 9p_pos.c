#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "config.h"
#include "9p.h"
#include "9p_pos.h"

uint16_t pos_global = 0;

int16_t pos_read(uint8_t oc, const struct DirectoryEntry *dp, uint16_t tag, uint64_t * offset, uint32_t * count)
{
	uint8_t reply[6];
	
	*((uint16_t *)(reply+4)) = pos_global;
	
	p9_send_reply(oc, Rread, tag, reply, 6);
	return 0;
}

DirectoryEntry dir_pos[3];
DirectoryEntry * p9_build_pos_dir(uint8_t parent_qid_index, DirectoryEntry * parent)
{
	dir_pos[0] = (DirectoryEntry){"..", {QTDIR, 0, parent_qid_index}, parent};
	dir_pos[1] = (DirectoryEntry){"0", {QTFILE, 0, p9_register_de(dir_pos+1)}, 0, pos_read, p9_nowrite};
	dir_pos[2] = (DirectoryEntry){0};
	
	return dir_pos;
}