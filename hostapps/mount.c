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
	if (argc < 3)
	{
		printf("mnt device mountpoint\n");
		return;
	}
	fd = open(argv[1], O_RDWR);
	if (fd < 0)
	{
		printf("open error\n");
		return;
	}
	err = tcflush(fd, TCIOFLUSH);
	if (err < 0)
	{
		printf("flush error\n");
		close(fd);
		return;
	}
	sprintf(options, "debug=0xffff,noextend,trans=fd,rfdno=%i,wfdno=%i", fd, fd);
	printf("%s\n", options);
	execv("/bin/mount", args);
}
