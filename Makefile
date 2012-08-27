AVRDUDE_PROGRAMMER	:= avrisp2
TARGET				:= fdioCV2.hex
ELF					:= fdioCV2.elf
SRCS				:= $(wildcard *.c)
CC					:= avr-gcc
OBJCOPY				:= avr-objcopy
OBJDUMP				:= avr-objdump
ADDRDEF				:= addrdef.txt

CCFLAGS = -std=c99 -mmcu=atmega1284p -O3 -Wall -fno-strict-aliasing
AVFLAGS = -c ${AVRDUDE_PROGRAMMER} -p m1284p -P usb
LDFLAGS = -Wl,--section-start=.boot=0x1F000
LIBS    = 
OCFLAGS = -j .text -j .data -j .boot -O ihex

.PHONY: all clean distclean 
all:: ${TARGET} 


${TARGET}: ${ELF}
	${OBJCOPY} ${OCFLAGS} $< $@
	@echo "-Tdata=\c" > ${ADDRDEF}
	${OBJDUMP} -t fdioCV2.elf | grep pgm_ram | awk '{printf $$1}' >> ${ADDRDEF}
	@echo " -Ttext=\c" >> ${ADDRDEF}
	${OBJDUMP} -t fdioCV2.elf | grep pgm_mem | awk '{print $$1}' >> ${ADDRDEF}
	
${ELF}: ${SRCS} 
	${CC} ${CCFLAGS} ${LDFLAGS} -o $@ ${SRCS} ${LIBS} 

clean:: 
	-rm -f *~ *.o *.dep ${TARGET} ${ELF}
program: ${TARGET}
	avrdude ${AVFLAGS} -U flash:w:${TARGET} -U eeprom:w:config.hex
		
eeprom:
	avrdude ${AVFLAGS} -U eeprom:w:config.hex
	
fuse:
	avrdude ${AVFLAGS} -U lfuse:w:0xC7:m -U hfuse:w:0xD9:m -U efuse:w:0xFF:m

distclean:: clean
