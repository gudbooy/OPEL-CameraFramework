#include <stdio.h>
#include<unistd.h>
#include<sys/ipc.h>
#include<sys/shm.h>
#include<stdio.h>
#include<string.h>

#include<getopt.h>
#include<fcntl.h>
#include<errno.h>
#include<sys/stat.h>
#include<sys/types.h>
#include<sys/time.h>
#include<sys/mman.h>
#include<sys/ioctl.h>
#include<sys/wait.h>
#include<semaphore.h>

int main()
{
	int fd;
	if(-1 == mkfifo("/tmp/fifo", 0666))
	{
		fprintf(stderr, "Failed to make FIFO\n");
//		exit(1);
		return -1;
	}
	fd = open("/tmp/fifo", O_RDWR | O_NONBLOCK, 0);

	for(;;)
	{
	
		write(fd, "hihihi", 7);
		sleep(5);
	}
	return 0;
}
