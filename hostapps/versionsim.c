#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <termios.h>

int main()
{
	int fd = -1;
	unsigned char data[] = {0x13,0x00, 0x00,0x00,0x64,0xFF,0xFF,0x00,0x20, 0x00, 0x00, 0x06, 0x39,0x50,0x32,0x30,0x30,0x30};
	int err = 0;
	fd = open("/dev/ttyS0", O_RDWR);
	if (fd < 0)
	{
		printf("open error\n");
		return;
	}
	printf("flushing\n");
	err = tcflush(fd, TCIOFLUSH);
	if (err < 0)
	{
		printf("flush error\n");
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
