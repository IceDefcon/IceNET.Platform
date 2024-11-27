#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <stdlib.h>

#define DEVICE_PATH "/dev/IceNETDisk0" // Adjust based on your ramdisk naming

int main() {
    const char *data = "Hello, RAM Disk!";
    const size_t data_size = strlen(data);
    char read_buf[128] = {0};

    // Open the block device for writing
    int fd = open(DEVICE_PATH, O_RDWR);
    if (fd < 0) {
        perror("Failed to open block device");
        return EXIT_FAILURE;
    }

#if 1 // Write data to the block device
    ssize_t written = write(fd, data, data_size);
    if (written < 0) {
        perror("Failed to write to block device");
        close(fd);
        return EXIT_FAILURE;
    }
    printf("Written to device: %s\n", data);
#endif

#if 0 // Seek back to the beginning of the device
    if (lseek(fd, 0, SEEK_SET) < 0) {
        perror("Failed to seek in block device");
        close(fd);
        return EXIT_FAILURE;
    }
#endif

#if 0 // Read data back from the block device
    ssize_t read_bytes = read(fd, read_buf, sizeof(read_buf) - 1);
    if (read_bytes < 0) {
        perror("Failed to read from block device");
        close(fd);
        return EXIT_FAILURE;
    }
    printf("Read from device: %s\n", read_buf);
#endif

    // Clean up
    close(fd);
    return EXIT_SUCCESS;
}
