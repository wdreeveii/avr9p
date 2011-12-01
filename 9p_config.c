#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "config.h"
#include "9p.h"
int16_t demowrite(const struct DirectoryEntry *dp, uint64_t *offset, uint32_t *count, uint8_t *data);
int16_t demoread(const struct DirectoryEntry *dp, uint16_t tag, uint64_t * offset, uint32_t * count);

int16_t read_usart0_baud(const struct DirectoryEntry *dp, uint16_t tag, uint64_t *offset, uint32_t * count)
{
	uint8_t reply[20];
	uint8_t * replyptr = reply + 4;
	
	uint8_t outlength = 0;
	*((uint32_t *)(reply)) = 0;
	
	if (*(offset) != 0)
		goto done;
	switch(config_get_baud(0))
	{
		case 1041:
			outlength = strlcpy((char*)replyptr, "2400\n", 16);
			break;
		case 520:
			outlength = strlcpy((char*)replyptr, "4800\n", 16);
			break;
		case 259:
			outlength = strlcpy((char*)replyptr, "9600\n", 16);
			break;
		case 173:
			outlength = strlcpy((char*)replyptr, "14400\n", 16);
			break;
		case 129:
			outlength = strlcpy((char*)replyptr, "19200\n", 16);
			break;
		case 86:
			outlength = strlcpy((char*)replyptr, "28800\n", 16);
			break;
		case 64:
			outlength = strlcpy((char*)replyptr, "38400\n", 16);
			break;
		case 42:
			outlength = strlcpy((char*)replyptr, "57600\n", 16);
			printf("strlcpy\n");
			break;
		case 32:
			outlength = strlcpy((char*)replyptr, "76800\n", 16);
			break;
		case 21:
			outlength = strlcpy((char*)replyptr, "115200\n", 16);
			break;
		case 10:
			outlength = strlcpy((char*)replyptr, "230400\n", 16);
			break;
		case 9:
			outlength = strlcpy((char*)replyptr, "250000\n", 16);
			break;
		case 4:
			outlength = strlcpy((char*)replyptr, "500000\n", 16);
			break;
	}
	*((uint32_t *)(reply)) = outlength;
	replyptr += outlength;
	
done:

	send_reply(Rread, tag, reply, replyptr - reply);
	return 0;
}

DirectoryEntry * p9_build_config_timer(uint8_t parent_qid, DirectoryEntry * parent)
{
	DirectoryEntry *dir_timer = (DirectoryEntry *)malloc(5 * sizeof(DirectoryEntry));
	if (!dir_timer)
	{
		printf("Dir build malloc fail\n");
		return 0;
	}
	*(dir_timer) = (DirectoryEntry){"..", {QTDIR, 0, parent_qid}, parent};
	*(dir_timer + 1) = (DirectoryEntry){"new_event", {QTFILE, 0, p9_register_de(dir_timer+1)}, 0, demoread, demowrite};
	*(dir_timer + 2) = (DirectoryEntry){"delete_event",{QTFILE, 0, p9_register_de(dir_timer+2)}, 0, demoread, demowrite};
	*(dir_timer + 3) = (DirectoryEntry){"list", {QTFILE, 0, p9_register_de(dir_timer+3)}, 0, demoread, demowrite};
	*(dir_timer + 4) = (DirectoryEntry){0};
	
	return dir_timer;
}

DirectoryEntry * p9_build_config(uint8_t parent_qid, DirectoryEntry * parent)
{
	uint8_t tmpde;
	DirectoryEntry *dir_config = (DirectoryEntry *)malloc(5 * sizeof(DirectoryEntry));
	if (!dir_config)
	{
		printf("Dir build malloc fail\n");
		return 0;
	}
	*(dir_config) = (DirectoryEntry){"..", {QTDIR, 0, parent_qid}, parent};
	*(dir_config + 1) = (DirectoryEntry){"usart0_baud", {QTFILE, 0, p9_register_de(dir_config+1)}, 0, read_usart0_baud, demowrite};
	*(dir_config + 2) = (DirectoryEntry){"usart1_baud", {QTFILE, 0, p9_register_de(dir_config+2)}, 0, demoread, demowrite};
	tmpde = p9_register_de(dir_config+3);
	*(dir_config + 3) = (DirectoryEntry){"timer", {QTDIR, 0, tmpde}, p9_build_config_timer(tmpde, dir_config)};
	*(dir_config + 4) = (DirectoryEntry){0};
	
	return dir_config;
}