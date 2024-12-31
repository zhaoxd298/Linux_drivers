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
	int ret;
	char buf[128] = {0};
	struct pollfd p_fds[5];  	/* 最大监听5个句柄 */
	int max_fd; 				/* 监听文件描述符中最大的文件号 */

	fd = open("/dev/poll_select_dev", O_RDWR);
	if (0 > fd) {
		perror("can not open poll_select_dev");
		return -1;
	}
	printf("open \"poll_select_dev\" success\n");

	/* poll 监听参数 */
	for (ret = 0; ret < 5; ret++) {
		p_fds[ret].fd = -1;
	}
	p_fds[0].fd = fd;
	p_fds[0].events = POLLIN | POLLRDNORM;
	p_fds[0].revents = 0;
	max_fd = 1;

	while (1) {
		ret = poll(p_fds, max_fd+1, -1);			/* -1表示阻塞，不超时 */
		if (ret < 0) {
			perror("poll error");
			close(fd);
		}

		if ((p_fds[0].revents & POLLIN) == POLLIN || (p_fds[0].revents & POLLRDNORM) == POLLRDNORM) {
			read(fd, &ret, sizeof(int));
			printf("read:%d\n", ret);
		}

		usleep(10000);
	}

	close(fd);   

	return 0;
}
