
// io control code
#ifndef _IOCONTROL_H
#define _IOCONTROL_H

#define NUM_PORTS 14
#define PORT_OUTPUT 0
#define PORT_DINPUT 2
#define PORT_AINPUT 4

void io_init();
void iocontrol(unsigned char port, unsigned char state);
void ioflip(unsigned char port);

#endif
