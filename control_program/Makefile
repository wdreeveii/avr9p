TARGET				:= pgm.bin
ELF					:= pgm.elf
SRCS				:= test.c
OBJECTS				:= $(SRCS:.c=.o)
CC					:= avr-gcc
LD					:= avr-ld
OBJCOPY				:= avr-objcopy

ADDRESS_FILE:=../addrdef.txt
ADDRESS_DEFINITIONS:=`cat $(ADDRESS_FILE)`

CCFLAGS = -std=c99 -mmcu=atmega1284p -O3 -Wall -fno-strict-aliasing -nostdlib
LDFLAGS = ${ADDRESS_DEFINITIONS} -m avr5 -nostdlib -N
LIBS    = 
OCFLAGS = -j .text -j .data -j .boot -O binary --pad-to 0x200

.PHONY: all clean distclean 
all:: ${TARGET} 


${TARGET}: ${ELF} ${ADDRESS_FILE}
	${OBJCOPY} ${OCFLAGS} $< $@
	
${ELF}: ${OBJECTS}
	${LD} ${LDFLAGS} -o $@ $< ${OBJS}
	
${OBJECTS}: ${SRCS}
	${CC} -c -o $@ $< ${CCFLAGS} 
	
clean:: 
	-rm -f *~ *.o *.dep ${TARGET} ${ELF}

distclean:: clean
