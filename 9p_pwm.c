#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <avr/io.h>

#include <util/atomic.h>

#include "config.h"
#include "9p.h"
#include "9p_pwm.h"

int16_t pwm0_write(const struct DirectoryEntry *dp, uint64_t *offset, uint32_t *count, uint8_t *indata)
{
	uint8_t datacpy[*count + 1];
	datacpy[*count] = 0;
	char port;
	int16_t prescale;
	memcpy(datacpy, indata, *count);
	if (sscanf((char *)datacpy, "set %c %i", &port, &prescale) == 2)
	{
		ATOMIC_BLOCK(ATOMIC_RESTORESTATE)
		{
			TCCR0A |= (1 << WGM01) | (1 << WGM00);
			
			switch(port)
			{
				case 'a':
					DDRB |= (1<<PB3);
					TCCR0A |= (1<<COM0A1);
					OCR0A = 0;
					break;
				case 'b':
					DDRB |= (1<<PB4);
					TCCR0A |= (1<<COM0B1);
					OCR0A = 0;
					break;
			}
			switch(prescale)
			{
				case -1:
					TCCR0B = 0;
					break;
				case 0:
					TCCR0B = (1 << CS00);
					break;
				case 8:
					TCCR0B = (1 << CS01);
					break;
				case 64:
					TCCR0B = (1 << CS00) | (1 << CS01);
					break;
				case 256:
					TCCR0B = (1 << CS02);
					break;
				case 1024:
					TCCR0B = (1 << CS00) | (1 << CS02);
					break;
				default:
					TCCR0B = 0;
					break;
			}
		}
	}
	else return -1;
	return *count;
}
int16_t pwm0a_write(const struct DirectoryEntry *dp, uint64_t *offset, uint32_t *count, uint8_t *indata)
{
	uint8_t datacpy[*count + 1];
	datacpy[*count] = 0;
	uint16_t duty;
	memcpy(datacpy, indata, *count);
	if (sscanf((char *)datacpy, "%u", &duty) == 1)
	{
		ATOMIC_BLOCK(ATOMIC_RESTORESTATE)
		{
			OCR0A = (duty & 0xFF);
		}
	}
	else return -1;
	return *count;
}
int16_t pwm0b_write(const struct DirectoryEntry *dp, uint64_t *offset, uint32_t *count, uint8_t *indata)
{
	uint8_t datacpy[*count + 1];
	datacpy[*count] = 0;
	uint16_t duty;
	memcpy(datacpy, indata, *count);
	if (sscanf((char *)datacpy, "%u", &duty) == 1)
	{
		ATOMIC_BLOCK(ATOMIC_RESTORESTATE)
		{
			OCR0B = (duty & 0xFF);
		}
	}
	else return -1;
	return *count;
}
int16_t pwm1_write(const struct DirectoryEntry *dp, uint64_t *offset, uint32_t *count, uint8_t *indata)
{
	uint8_t datacpy[*count + 1];
	datacpy[*count] = 0;
	time_t newtime;
	memcpy(datacpy, indata, *count);
	if (sscanf((char *)datacpy, "%lu", &newtime) == 1)
	{
		set_time(newtime);
	}
	return *count;
}
int16_t pwm2_write(const struct DirectoryEntry *dp, uint64_t *offset, uint32_t *count, uint8_t *indata)
{
	uint8_t datacpy[*count + 1];
	datacpy[*count] = 0;
	time_t newtime;
	memcpy(datacpy, indata, *count);
	if (sscanf((char *)datacpy, "%lu", &newtime) == 1)
	{
		set_time(newtime);
	}
	return *count;
}

DirectoryEntry dir_pwm[11];
DirectoryEntry * p9_build_pwm_dir(uint8_t parent_qid_index, DirectoryEntry * parent)
{
	dir_pwm[0]= (DirectoryEntry){"..", {QTDIR, 0, parent_qid_index}, parent};
	dir_pwm[1] = (DirectoryEntry){"0", {QTFILE, 0, p9_register_de(dir_pwm+1)}, 0, p9_noread, pwm0_write};
	dir_pwm[2] = (DirectoryEntry){"0a", {QTFILE, 0, p9_register_de(dir_pwm+2)}, 0, p9_noread, pwm0a_write};
	dir_pwm[3] = (DirectoryEntry){"0b", {QTFILE, 0, p9_register_de(dir_pwm+3)}, 0, p9_noread, pwm0b_write};
	dir_pwm[4] = (DirectoryEntry){"1", {QTFILE, 0, p9_register_de(dir_pwm+4)}, 0, p9_noread, p9_nowrite};
	dir_pwm[5] = (DirectoryEntry){"1a", {QTFILE, 0, p9_register_de(dir_pwm+5)}, 0, p9_noread, p9_nowrite};
	dir_pwm[6] = (DirectoryEntry){"1b", {QTFILE, 0, p9_register_de(dir_pwm+6)}, 0, p9_noread, p9_nowrite};
	dir_pwm[7] = (DirectoryEntry){"2", {QTFILE, 0, p9_register_de(dir_pwm+7)}, 0, p9_noread, p9_nowrite};
	dir_pwm[8] = (DirectoryEntry){"2a", {QTFILE, 0, p9_register_de(dir_pwm+8)}, 0, p9_noread, p9_nowrite};
	dir_pwm[9] = (DirectoryEntry){"2b", {QTFILE, 0, p9_register_de(dir_pwm+9)}, 0, p9_noread, p9_nowrite};
	dir_pwm[10] = (DirectoryEntry){0};
	
	return dir_pwm;
}