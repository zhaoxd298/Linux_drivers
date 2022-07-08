#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <stdio.h>
#include <stdlib.h> 
#include <string.h> 
#include <unistd.h>
#include <poll.h>  

int main(int argc, char *argv[])
{
	int fd;
	int i = 0;

	fd = open("/dev/poll_select_dev", O_RDWR);
	if (0 > fd) {
		perror("can not open poll_select_dev");
		return -1;
	}
	printf("open \"poll_select_dev\" success\n");

	while (1) {
		write(fd, &i, sizeof(int));
		i++;

		usleep(5000);
	}

	close(fd);   

	return 0;
}
