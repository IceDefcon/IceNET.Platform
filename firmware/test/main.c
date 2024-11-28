#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <stdlib.h>

#define DEVICE_PATH "/dev/IceNETDisk0" // Adjust based on your ramdisk naming

//
// 0x40
//
// For Normal mode
//
// acc_us = 0b0 (undersampling disabled)
// acc_bwp = 0b010 (normal mode)
// acc_odr = 0b1100 (1600Hz)
//

typedef struct 
{
    char header;    /* Unique ID of operation */
    char size;      /* Total bytes sent to FPGA in one SPI/DMA Transfer */
    char ctrl;      /* Interface (i2c, spi, pwm), Read or Write */
    char devId;     /* In case of i2c */
    char ops;       /* Number of Read or Write operations */
    char checksum;  /* 8-Bit checksum */
    char payload[]; /* Combined register addresses and write data */
} OperationType;


OperationType* createOperation(char devId, char ctrl, char ops) 
{
    char regSize = ops;
    char dataSize = ops;
    char totalSize = sizeof(OperationType) + regSize + dataSize;

    // Allocate memory
    OperationType* op = (OperationType*)malloc(totalSize);
    
    if (!op)
    {
        return NULL;
    } 

    op->header = 0x7E;          /* Write config ID */
    op->size = (char)totalSize; /* Bytes to send to FPGA */
    op->ctrl = ctrl;            /* 0:i2c 1:Write */
    op->devId = devId;          /* BMI160 Id */
    op->ops = ops;              /* Number of writes */
    op->checksum = 0;           /* TODO */

    // Initialize payload (zero out memory)
    memset(op->payload, 0, regSize + dataSize);

    return op;
}


int main() 
{
    size_t ops = 2;

    OperationType* op = createOperation(0x69, 0x01, 0x02);
    
    if (!op) 
    {
        perror("Failed to allocate operation");
        return EXIT_FAILURE;
    }

    char* reg = op->payload;            // Register array
    char* data = op->payload + ops;     // Data array (offset by ops)

    /**
     * This is size specific piece of code
     * For size 2 we have 2x Regs +2x Data
     */
    *reg[0] = 0x40;
    *data[0] = 0x2C;

    *reg[1] = 0x40;
    *data[1] = 0x2C;

    const char *data = "Hello, RAM Disk!";
    const size_t data_size = strlen(data);
    char read_buf[128] = {0};

    // Open the block device for writing
    int fd = open(DEVICE_PATH, O_RDWR);
    if (fd < 0) 
    {
        perror("Failed to open block device");
        return EXIT_FAILURE;
    }

#if 1 // Write data to the block device
    ssize_t written = write(fd, data, data_size);
    if (written < 0) 
    {
        perror("Failed to write to block device");
        close(fd);
        return EXIT_FAILURE;
    }
    printf("Written to device: %s\n", data);
#endif

#if 0 // Seek back to the beginning of the device
    if (lseek(fd, 0, SEEK_SET) < 0) 
    {
        perror("Failed to seek in block device");
        close(fd);
        return EXIT_FAILURE;
    }
#endif

#if 0 // Read data back from the block device
    ssize_t read_bytes = read(fd, read_buf, sizeof(read_buf) - 1);
    if (read_bytes < 0) 
    {
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
