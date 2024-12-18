#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <stdlib.h>

#define DEVICE_PATH "/dev/IceNETDisk0" // Adjust based on your ramdisk naming
#define SECTOR_SIZE 512  // Sector size in bytes

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

// Helper function to calculate checksum
char calculate_checksum(char *data, size_t size) 
{
    char checksum = 0;
    for (size_t i = 0; i < size; i++) {
        checksum ^= data[i];
    }
    return checksum;
}

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

    op->header = 0x1E;          /* Write config ID */
    op->size = (char)totalSize; /* Bytes to send to FPGA */
    op->ctrl = ctrl;            /* 0:i2c 1:Write */
    op->devId = devId;          /* BMI160 Id */
    op->ops = ops;              /* Number of writes */

    // Initialize payload (zero out memory)
    memset(op->payload, 0, totalSize - sizeof(OperationType));

    // Calculate checksum
    op->checksum = calculate_checksum((char*)op, totalSize - 1);

    return op;
}

int main() 
{
    ssize_t bytes = 0;
    int sector_offset = 0;
    char ops = 2; /* Set up 2 registers only */

    // Allocate memory for the first sector
    char first_sector[1] = {2};
    char forth_sector[4] = {0x00, 0x69, 0x00, 0x00};

    // [1] Sector
    OperationType* dev_0_op = createOperation(0x69, 0x01, ops);
    if (!dev_0_op) 
    {
        perror("Failed to allocate operation");
        return EXIT_FAILURE;
    }

    char* dev_0_reg = dev_0_op->payload;
    char* dev_0_bin = dev_0_op->payload + ops;

    dev_0_reg[0] = 0x7E;    /* CMD */
    dev_0_bin[0] = 0x11;    /* Set PMU mode of accelerometer to normal */
    dev_0_reg[1] = 0x40;    /* ACC_CONF */
    dev_0_bin[1] = 0x2C;    /* acc_bwp = 0x2 normal mode + acc_od = 0xC 1600Hz r*/

    // [2] Sector
    OperationType* dev_1_op = createOperation(0x53, 0x01, ops);
    if (!dev_1_op) 
    {
        perror("Failed to allocate operation");
        free(dev_0_op);
        return EXIT_FAILURE;
    }

    char* dev_1_reg = dev_1_op->payload;
    char* dev_1_bin = dev_1_op->payload + ops;

    dev_1_reg[0] = 0x2D;    /* CMD */
    dev_1_bin[0] = 0x08;    /* Set PMU mode of accelerometer to normal */
    dev_1_reg[1] = 0x31;    /* ACC_CONF */
    dev_1_bin[1] = 0x00;    /* acc_bwp = 0x2 normal mode + acc_od = 0xC 1600Hz r*/

    if ((dev_0_op->size > SECTOR_SIZE) || (dev_1_op->size > SECTOR_SIZE)) 
    {
        fprintf(stderr, "Operation size exceeds sector size (%d bytes)\n", SECTOR_SIZE);
        free(dev_0_op);
        free(dev_1_op);
        return EXIT_FAILURE;
    }

    int fd = open(DEVICE_PATH, O_RDWR);
    if (fd < 0) 
    {
        perror("Failed to open block device");
        free(dev_0_op);
        free(dev_1_op);
        return EXIT_FAILURE;
    }

    // Write to sector 0
    bytes = write(fd, first_sector, sizeof(first_sector));
    if (bytes < 0) 
    {
        perror("Failed to write to block device");
        close(fd);
        free(dev_0_op);
        free(dev_1_op);
        return EXIT_FAILURE;
    }
    printf("Write %ld Bytes to ramDisk to Sector 0\n", bytes);

    // Seek to sector 1 (sector_offset = 1)
    lseek(fd, SECTOR_SIZE * 1, SEEK_SET);

    // Write to sector 1
    bytes = write(fd, dev_0_op->payload, dev_0_op->size);
    if (bytes < 0) 
    {
        perror("Failed to write to sector 1");
        close(fd);
        free(dev_0_op);
        free(dev_1_op);
        return EXIT_FAILURE;
    }
    printf("Write %ld Bytes to ramDisk to Sector 1\n", bytes);

    // Seek to sector 2 (sector_offset = 2)
    lseek(fd, SECTOR_SIZE * 2, SEEK_SET);

    // Write to sector 2
    bytes = write(fd, dev_1_op->payload, dev_1_op->size);
    if (bytes < 0) 
    {
        perror("Failed to write to sector 2");
        close(fd);
        free(dev_0_op);
        free(dev_1_op);
        return EXIT_FAILURE;
    }
    printf("Write %ld Bytes to ramDisk to Sector 2\n", bytes);

    // Seek to sector 2 (sector_offset = 2)
    lseek(fd, SECTOR_SIZE * 3, SEEK_SET);

    // Write to sector 2
    bytes = write(fd, forth_sector, sizeof(forth_sector));
    if (bytes < 0) 
    {
        perror("Failed to write to sector 3");
        close(fd);
        free(dev_0_op);
        free(dev_1_op);
        return EXIT_FAILURE;
    }
    printf("Write %ld Bytes to ramDisk to Sector 3\n", bytes);

    close(fd);
    free(dev_0_op);
    free(dev_1_op);
    return EXIT_SUCCESS;
}
