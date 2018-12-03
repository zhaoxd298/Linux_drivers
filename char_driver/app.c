#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <stdio.h>
#include <stdlib.h> 
#include <string.h> 

int main(int argc, char *argv[])
{
    int fd;
    char buf[128] = {0};
    
    fd = open("/dev/char_driver", O_RDWR);
    if(0 > fd){
    	perror("can not open 6410_led:");
    }

    write(fd, "hello driver!", 13);

    lseek(fd, SEEK_SET, 0);

    read(fd, buf, 5);
    printf("%s\n", buf);

    memset(buf, 0, sizeof(buf));

    read(fd, buf, 10);
    printf("%s\n", buf);

    close(fd);   

    return 0;
}
