#include <stdio.h>      // For printf, perror
#include <fcntl.h>      // For open, O_RDWR
#include <unistd.h>     // For close, sleep
#include <errno.h>      // For errno (optional)

int main()
{
    int fd = open("/dev/KernelBlock", O_RDWR);

    if (fd < 0)
    {
        perror("Failed to open device");
        return 1;
    }

    sleep(1);
    close(fd);
    printf("Device closed.\n");

    return 0;
}

