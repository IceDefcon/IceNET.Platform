#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>

int main() {
    int fd;
    char buffer[512]; // Assuming block size is 512 bytes

    // Open the block device
    fd = open("/dev/iceBLOCK", O_RDWR);
    if (fd == -1) {
        perror("Failed to open block device");
        return 1;
    }

    // Read data from the block device
    if (read(fd, buffer, sizeof(buffer)) == -1) {
        perror("Failed to read from block device");
        close(fd);
        return 1;
    }

    // Write data to the block device
    // Example: Write "Hello" to the first block
    char *data = "Hello";
    if (write(fd, data, strlen(data)) == -1) {
        perror("Failed to write to block device");
        close(fd);
        return 1;
    }

    // Close the block device
    close(fd);

    return 0;
}
