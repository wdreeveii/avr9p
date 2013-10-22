#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <termios.h>

int main(int argc, char *argv[])
{
	int fd = -1;
	unsigned char data[] = {0x13,0x00, 0x00,0x00,0x64,0xFF,0xFF,0x00,0x20, 0x00, 0x00, 0x06, 0x39,0x50,0x32,0x30,0x30,0x30};
	int err = 0;
	uint32_t baud;
		
	speed_t devicespeed = B57600;
	struct termios serialmode;
	struct termios confirmation;
	
	if (argc < 3)
	{
		printf("versionsim <device> <speed>\n");
		return;
	}
	if (sscanf(argv[2], "%u", &baud) != 1)
	{
		printf("Incorrect baud setting please try again\n");
		return;
	}
	switch(baud)
	{
		case 2400: devicespeed = B2400; break;
		case 4800: devicespeed = B4800; break;
		case 9600: devicespeed = B9600; break;
		case 19200: devicespeed = B19200; break;
		case 38400: devicespeed = B38400; break;
		case 57600: devicespeed = B57600; break;
		case 115200: devicespeed = B115200; break;
		case 230400: devicespeed = B230400; break;
		case 500000: devicespeed = B500000; break;
		default: printf("Incorrect baud setting please try again\n");
				return;
		
	}
	// open
	fd = open(argv[1], O_RDWR);
	if (fd < 0)
	{
		printf("open error\n");
		return;
	}
	// flush
	err = tcflush(fd, TCIOFLUSH);
	if (err < 0)
	{
		printf("flush error\n");
		close(fd);
		return;
	}
	// get terminal modes
	err = tcgetattr(fd, &serialmode);
	if (err < 0)
	{
		printf("tcgetattr error\n");
		close(fd);
		return;
	}
	// configure to raw
	serialmode.c_iflag &= ~(IGNBRK | BRKINT | IGNPAR | PARMRK | INPCK | ISTRIP | INLCR | IGNCR | ICRNL | IXON | IXOFF | IUCLC | IXANY | IMAXBEL | IUTF8);
	serialmode.c_oflag &= ~(OPOST | OLCUC | OCRNL | ONLCR | ONOCR | ONLRET | OFILL | OFDEL );
	serialmode.c_lflag &= ~(ISIG | ICANON | IEXTEN | ECHO | ECHOE | ECHOK | ECHONL | NOFLSH | XCASE | TOSTOP | ECHOPRT | ECHOCTL | ECHOKE);
	serialmode.c_cflag &= ~(CSIZE | PARENB | PARODD | HUPCL | CSTOPB | CRTSCTS);
	serialmode.c_cflag |= (CS8 | CREAD | CLOCAL);
	err = cfsetspeed(&serialmode, devicespeed);
	if (err < 0)
	{
		printf("cfsetspeed error\n");
		close(fd);
		return;
	}
	// set new terminal modes
	err = tcsetattr(fd, TCSANOW, &serialmode);
	if (err < 0)
	{
		printf("tcsetattr error \n");
		close(fd);
		return;
	}
	printf("writing\n");
	err = write(fd, data, 0x13);
	if (err < 0)
	{
		printf("write error\n");
		close(fd);
		return;
	}
	printf("reading\n");
	err = read(fd, data, 7);
	if (err < 0)
	{
		printf("read error\n");
		close(fd);
		return;
	}
	close(fd);
	for (fd = 0; fd < err; fd++)
	{	
		printf("%x|",*((unsigned char *)(data + fd)) );
	}
	printf("\n");
}
