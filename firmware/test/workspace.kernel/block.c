#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>

int main() 
{
    int fd;
    char buffer[512]; // Assuming block size is 512 bytes

    // Open the block device
    fd = open("/dev/iceBLOCK", O_RDWR);
    if (fd == -1) 
    {
        perror("Failed to open block device");
        return 1;
    }

    // Close the block device
    close(fd);

    return 0;
}