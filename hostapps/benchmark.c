#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <string.h>
#include <errno.h>
#include <stdint.h>
#include <time.h>

int main(int argc, char *argv[])
{
	errno = 0;
	int fd = -1;
	int err = 0;
	char buf[3] = {0};
	int numrevs = 0;
	time_t before;
	time_t after;
	if (argc < 2)
	{
		printf("%s file_to_test\n", argv[0]);
		return;
	}
	
	fd = open(argv[1], O_RDWR);
	if (fd < 0)
	{
		printf("open error: %s\n", strerror(errno));
		return;
	}
	
	before = time(NULL);
	for (numrevs = 0; numrevs < 10000; numrevs++)
	{
		err = read(fd, buf, 2);
		if (err < 0)
		{
			printf("read error: %s\n", strerror(errno));
			close(fd);
			return;
		}
		//printf("%x|%x\n", (uint8_t)*(buf + 1), (uint8_t)*buf);
	}
	after = time(NULL);
	printf("reads per seconds: 10000/%li = %f\n", after - before, (double)(10000.0/(after - before)));
	close(fd);
	return;
}