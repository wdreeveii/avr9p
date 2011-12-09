#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <termios.h>

int main(int argc, char *argv[])
{
	int fd = -1;
	int err = 0;
	char options[200];
	char *args[] = {"/bin/mount", "-v", "-t", "9p", "-o", options, argv[1], argv[2], (char*)NULL};
	
	speed_t devicespeed = B57600;
	struct termios serialmode;
	struct termios confirmation;
	
	if (argc < 3)
	{
		printf("mnt device mountpoint\n");
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
	// set terminal modes
	err = tcgetattr(fd, &serialmode);
	if (err < 0)
	{
		printf("tcgetattr error\n");
		close(fd);
		return;
	}

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
	err = tcsetattr(fd, TCSANOW, &serialmode);
	if (err < 0)
	{
		printf("tcsetattr error \n");
		close(fd);
		return;
	}
	sprintf(options, "debug=0xffff,noextend,trans=fd,rfdno=%i,wfdno=%i", fd, fd);
	execv("/bin/mount", args);
}