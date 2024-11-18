#include <stdio.h>      // For printf, perror
#include <fcntl.h>      // For open, O_RDWR
#include <unistd.h>     // For close, sleep, write
#include <errno.h>      // For errno (optional)

#define DATA_FILE "data"  // The file containing the data

int main()
{
    // Open the block device
    int dev_fd = open("/dev/KernelBlock", O_RDWR);
    if (dev_fd < 0)
    {
        perror("Failed to open block device");
        return 1;
    }

    // Open the data file for reading
    int file_fd = open(DATA_FILE, O_RDONLY);
    if (file_fd < 0)
    {
        perror("Failed to open data file");
        close(dev_fd);
        return 1;
    }

    // Read the data from the file into a buffer
    char buffer[1024];  // Adjust the buffer size as needed
    ssize_t bytes_read;
    while ((bytes_read = read(file_fd, buffer, sizeof(buffer))) > 0)
    {
        // Write the data read from the file to the block device
        ssize_t bytes_written = write(dev_fd, buffer, bytes_read);
        if (bytes_written < 0)
        {
            perror("Failed to write to block device");
            close(file_fd);
            close(dev_fd);
            return 1;
        }
        printf("Wrote %zd bytes to the device\n", bytes_written);
    }

    if (bytes_read < 0)
    {
        perror("Failed to read data file");
        close(file_fd);
        close(dev_fd);
        return 1;
    }

    printf("Data from file successfully written to the device\n");

    // Close the file and block device
    close(file_fd);
    close(dev_fd);
    printf("Device closed.\n");

    return 0;
}
