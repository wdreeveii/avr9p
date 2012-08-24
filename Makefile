AVRDUDE_PROGRAMMER	:= usbtiny
TARGET				:= fdioCV2.hex
ELF					:= fdioCV2.elf
SRCS				:= $(wildcard *.c)
CC					:= avr-gcc
OBJCOPY				:= avr-objcopy


CCFLAGS = -std=c99 -mmcu=atmega1284p -O3 -Wall -fno-strict-aliasing
AVFLAGS = -c ${AVRDUDE_PROGRAMMER} -p m1284p
LDFLAGS = -Wl,--section-start=.boot=0xF000
LIBS    = 
OCFLAGS = -j .text -j .data -j .boot -O ihex

.PHONY: all clean distclean 
all:: ${TARGET} 


${TARGET}: ${ELF}
	${OBJCOPY} ${OCFLAGS} $< $@
	
${ELF}: ${SRCS} 
	${CC} ${CCFLAGS} ${LDFLAGS} -o $@ ${SRCS} ${LIBS} 

clean:: 
	-rm -f *~ *.o *.dep ${TARGET} ${ELF}
program: ${TARGET}
	avrdude ${AVFLAGS} -U flash:w:${TARGET} -U eeprom:w:config.hex
		
eeprom:
	avrdude ${AVFLAGS} -U eeprom:w:config.hex
	
fuse:
	avrdude ${AVFLAGS} -U lfuse:w:0xc7:m -U efuse:w:0xff:m -U hfuse:w:0x99:m

distclean:: clean
