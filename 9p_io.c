#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <avr/io.h>

#include <util/atomic.h>

#include "config.h"
#include "9p.h"
#include "9p_io.h"
#include "iocontrol.h"

int16_t setio_write(const struct DirectoryEntry *dp, uint64_t *offset, uint32_t *count, uint8_t *indata)
{
	uint8_t datacpy[*count + 1];
	datacpy[*count] = 0;
	uint8_t port, state;
	memcpy(datacpy, indata, *count);
	if (sscanf((char *)datacpy, "%c %c", &port, &state) == 2)
	{
		ATOMIC_BLOCK(ATOMIC_RESTORESTATE)
		{
			iocontrol(port, state);
		}
	}
	else return -1;
	return *count;
}
int16_t flipio_write(const struct DirectoryEntry *dp, uint64_t *offset, uint32_t *count, uint8_t *indata)
{
	uint8_t datacpy[*count + 1];
	datacpy[*count] = 0;
	uint8_t port;
	memcpy(datacpy, indata, *count);
	if (sscanf((char *)datacpy, "%c", &port) == 2)
	{
		ATOMIC_BLOCK(ATOMIC_RESTORESTATE)
		{
			ioflip(port);
		}
	}
	else return -1;
	return *count;
}

DirectoryEntry * p9_build_io_dir(uint8_t parent_qid_index, DirectoryEntry * parent)
{
	DirectoryEntry *dir_io = (DirectoryEntry *)malloc(4 * sizeof(DirectoryEntry));
	if (!dir_io)
	{
		printf("Dir build malloc fail\n");
		return 0;
	}
	
	*dir_io = (DirectoryEntry){"..", {QTDIR, 0, parent_qid_index}, parent};
	*(dir_io + 1) = (DirectoryEntry){"setio", {QTFILE, 0, p9_register_de(dir_io+1)}, 0, p9_noread, setio_write};
	*(dir_io + 2) = (DirectoryEntry){"flipio", {QTFILE, 0, p9_register_de(dir_io+1)},0, p9_noread, flipio_write};
	*(dir_io + 3) = (DirectoryEntry){0};
	
	return dir_io;
}