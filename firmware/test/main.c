#include <stdio.h>      // For printf, perror
#include <fcntl.h>      // For open, O_RDWR
#include <unistd.h>     // For close, sleep, write
#include <errno.h>      // For errno (optional)

#define DEVICE_PATH "/dev/KernelBlock"  // The block device path
#define DATA "0123456789ABCDEF"         // The data to send

int main()
{
    // Open the block device
    int dev_fd = open(DEVICE_PATH, O_RDWR);
    if (dev_fd < 0)
    {
        perror("Failed to open block device");
        return 1;
    }

    // Prepare the data to write
    char *buffer = DATA;
    ssize_t bytes_written;
    size_t data_len = strlen(DATA);  // Length of the string

    // Write the data to the block device
    bytes_written = write(dev_fd, buffer, data_len);
    if (bytes_written < 0)
    {
        perror("Failed to write to block device");
        close(dev_fd);
        return 1;
    }
    printf("Wrote %zd bytes to the device: %s\n", bytes_written, buffer);

    printf("Data successfully written to the device\n");

    // Close the block device
    close(dev_fd);
    printf("Device closed.\n");

    return 0;
}
