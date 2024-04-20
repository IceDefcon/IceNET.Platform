#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>

#define DEVICE_PATH "/dev/iceBLOCK"
#define BUFFER_SIZE 512

int main() {
    int fd;
    char read_buffer[BUFFER_SIZE];
    char write_buffer[BUFFER_SIZE];
    ssize_t bytes_read, bytes_written;

    // Open the block device
    fd = open(DEVICE_PATH, O_RDWR);
    if (fd == -1) {
        perror("Failed to open the device");
        return 1;
    }

    // Read from the block device
    bytes_read = read(fd, read_buffer, BUFFER_SIZE);
    if (bytes_read == -1) {
        perror("Failed to read from the device");
        close(fd);
        return 1;
    }
    printf("Read %zd bytes from the device\n", bytes_read);

    // Display the read data
    printf("Read data: %s\n", read_buffer);

    // Write to the block device
    strcpy(write_buffer, "Hello, Block Device!");
    bytes_written = write(fd, write_buffer, strlen(write_buffer));
    if (bytes_written == -1) {
        perror("Failed to write to the device");
        close(fd);
        return 1;
    }
    printf("Written %zd bytes to the device\n", bytes_written);

    // Close the block device
    close(fd);

    return 0;
}
