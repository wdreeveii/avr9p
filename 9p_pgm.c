#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <util/atomic.h>
#include <util/crc16.h>
#include <avr/pgmspace.h>
#include <avr/boot.h>
#include <avr/interrupt.h>

#include "9p.h"
#include "config.h"

__attribute__ ((aligned (256))) uint8_t pgm_mem[16384] PROGMEM = {0};
uint8_t pgm_ram[2048] = {0};

static __inline__ void pgm_do_write( uint32_t page, uint8_t *buf ) __attribute__ ((section (".boot")));

static __inline__ void pgm_do_write( uint32_t page, uint8_t *buf )
{
	uint16_t i;
	uint8_t sreg;

	// Disable interrupts.
	
	sreg = SREG;
	cli();
	
	eeprom_busy_wait ();
	
	boot_page_erase (page);
	boot_spm_busy_wait ();      // Wait until the memory is erased.
	
	for (i=0; i<SPM_PAGESIZE; i+=2)
	{
		// Set up little-endian word.
		
		uint16_t w = *buf++;
		w += (*buf++) << 8;
		
		boot_page_fill (page + i, w);
	}
	
	boot_page_write (page);     // Store buffer in flash page.
	boot_spm_busy_wait();       // Wait until the memory is written.
	
	// Reenable RWW-section again. We need this if we want to jump back
	// to the application after bootloading.
	
	boot_rww_enable ();
	
	// Re-enable interrupts (if they were ever enabled).
	
	SREG = sreg;
} 

int16_t pgm_load(const struct DirectoryEntry *dp, uint64_t *offset, uint32_t *count, uint8_t *indata) __attribute__ ((section (".boot")));

int16_t pgm_load(const struct DirectoryEntry *dp, uint64_t *offset, uint32_t *count, uint8_t *indata)
{
	static uint8_t pgm_buffer[SPM_PAGESIZE];
	// place null terminated string in datacpy
	uint8_t datacpy[*count + 1];
	uint8_t *pageptr;
	uint16_t buflen = 0xFFFF;
	uint16_t index;
	uint32_t page;
	uint16_t checksum;
	
	if (*offset < SPM_PAGESIZE)
	{
		buflen = *count;
		if (*offset + *count > SPM_PAGESIZE)
			buflen = SPM_PAGESIZE - *offset;
		memcpy(pgm_buffer + (*offset), indata, buflen);
		return buflen;
	}
		
	memcpy(datacpy, indata, *count);
	datacpy[*count] = 0;
	
	checksum = strtoul((char*)datacpy, (char**)&pageptr, 10);
	page = strtoul((char*)pageptr, NULL, 10);
	
	for (index = 0; index < SPM_PAGESIZE; index++)
		buflen = _crc16_update(buflen, pgm_buffer[index]);
	
	if (checksum != buflen)
		return -1;

	pgm_do_write(((uint16_t)pgm_mem) + (page<<8), pgm_buffer );

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