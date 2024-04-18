#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>

#define DEVICE_PATH "/dev/my_block_device"
#define BUFFER_SIZE 512

int main() {
    int fd;
    char write_buffer[BUFFER_SIZE];
    char read_buffer[BUFFER_SIZE];

    // Open the block device
    fd = open(DEVICE_PATH, O_RDWR);
    if (fd < 0) {
        perror("Failed to open block device");
        return 1;
    }

    // Write data to the block device
    memset(write_buffer, 'A', BUFFER_SIZE);
    if (write(fd, write_buffer, BUFFER_SIZE) != BUFFER_SIZE) {
        perror("Failed to write to block device");
        close(fd);
        return 1;
    }

    // Read data from the block device
    if (read(fd, read_buffer, BUFFER_SIZE) != BUFFER_SIZE) {
        perror("Failed to read from block device");
        close(fd);
        return 1;
    }

    // Print the read data
    printf("Read from block device: %.*s\n", BUFFER_SIZE, read_buffer);

    // Close the block device
    close(fd);

    return 0;
}
