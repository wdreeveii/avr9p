#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <avr/io.h>

#include "config.h"
#include "9p.h"
#include "9p_pwm.h"

/*int16_t demowrite(const struct DirectoryEntry *dp, uint64_t *offset, uint32_t *count, uint8_t *indata)
{
	uint8_t index;
	for(index = 0; index < *count; index++)
	{
		printf("%x|", *(indata+index));	
	}
	printf("\n");
	return *count;
}*/

int16_t pwmwrite(const struct DirectoryEntry *dp, uint64_t *offset, uint32_t *count, uint8_t *indata)
{
	if (*count > 0)
		OCR0A = *indata;
	return *count;
}

DirectoryEntry * p9_build_pwm_dir(uint8_t parent_qid_index, DirectoryEntry * parent)
{
	DirectoryEntry *dir_pwm = (DirectoryEntry *)malloc(3 * sizeof(DirectoryEntry));
	if (!dir_pwm)
	{
		printf("Dir build malloc fail\n");
		return 0;
	}
	
	DDRB = (1<<PB3) | (1<<PB4);
	TCCR0A = (1 << WGM01) | (1 << WGM00) | (1<<COM0A1);
	TCCR0B = (1<<CS00) /*| (1 << CS01)*/;
	
	*dir_pwm = (DirectoryEntry){"..", {QTDIR, 0, parent_qid_index}, parent};
	*(dir_pwm + 1) = (DirectoryEntry){"0", {QTFILE, 0, p9_register_de(dir_pwm+1)}, 0, p9_noread, pwmwrite};
	*(dir_pwm + 2) = (DirectoryEntry){0};
	
	return dir_pwm;
}