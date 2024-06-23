#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <string.h>
#include <stdlib.h>
#include "pchar_ioctl.h"

int main(int argc, char *argv[])
{
    int fd, ret;

    if (argc < 2)
    {
        printf("Invalid usage.\n");
        printf("Usage1: %s clear\n", argv[0]);
        printf("Usage2: %s info\n", argv[0]);
        printf("Usage3: %s resize <new_size>\n", argv[0]);
        _exit(3);
    }

    fd = open("/dev/pchar0", O_RDWR);
    if (fd < 0)
    {
        perror("open() failed");
        _exit(1);
    }

    if (strcmp(argv[1], "clear") == 0)
    {
        ret = ioctl(fd, FIFO_CLEAR);
        if (ret != 0)
            perror("ioctl() failed");
        else
            printf("FIFO cleared.\n");
    }
    else if (strcmp(argv[1], "info") == 0)
    {
        info_t info;
        ret = ioctl(fd, FIFO_INFO, &info);
        if (ret != 0)
            perror("ioctl() failed");
        else
            printf("FIFO info: size=%d, filled=%d, empty=%d.\n", info.size, info.len, info.avail);
    }
    else if (strcmp(argv[1], "resize") == 0)
    {
        if (argc < 3)
        {
            printf("Invalid usage.\n");
            printf("Usage3: %s resize <new_size>\n", argv[0]);
            _exit(3);
        }

        long new_size = atol(argv[2]);

        ret = ioctl(fd, FIFO_RESIZE, &new_size);
        if (ret != 0)
            perror("ioctl() failed");
        else
            printf("FIFO resized to new size: %ld.\n", new_size);
    }
    else
    {
        printf("Invalid usage.\n");
        printf("Usage1: %s clear\n", argv[0]);
        printf("Usage2: %s info\n", argv[0]);
        printf("Usage3: %s resize <new_size>\n", argv[0]);
    }

    close(fd);
    return 0;
}

