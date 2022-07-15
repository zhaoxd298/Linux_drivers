#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <stdio.h>
#include <stdlib.h> 
#include <string.h> 
#include <unistd.h>

int main(int argc, char *argv[])
{
    int fd;
    
    fd = open("/dev/timer_dev", O_RDWR);
    if(0 > fd){
    	perror("can not open timer_dev:");
        return -1;
    }

    printf("open timer_dev success!\n");

    sleep(10);

    close(fd);  

    printf("close timer_dev success!\n"); 

    return 0;
}
