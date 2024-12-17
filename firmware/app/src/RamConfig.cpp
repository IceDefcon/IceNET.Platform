/*!
 * 
 * Author: Ice.Marek
 * iceNET Technology 2025
 * 
 */
#include <cstring>
#include <iostream>
#include <fcntl.h>
#include <unistd.h>
#include "RamConfig.h"

RamConfig::RamConfig()
{
    std::cout << "[INFO] [CONSTRUCTOR] " << this << " :: Instantiate RamConfig" << std::endl;
}

RamConfig::~RamConfig() 
{
    std::cout << "[INFO] [DESTRUCTOR] " << this << " :: Destroy RamConfig" << std::endl;
}

char RamConfig::calculate_checksum(char *data, size_t size) 
{
    char checksum = 0;
    
    for (size_t i = 0; i < size; i++) 
    {
        checksum ^= data[i];
    }

    return checksum;
}

RamConfig::OperationType* RamConfig::createOperation(char devId, char ctrl, char ops) 
{
    char regSize = ops;
    char dataSize = ops;
    char totalSize = sizeof(RamConfig::OperationType) + regSize + dataSize;

    // Allocate memory
    RamConfig::OperationType* op = (RamConfig::OperationType*)malloc(totalSize);

    if (!op)
    {
        perror("Failed to allocate operation");
        return NULL;
    } 

    op->header = 0x1E;              /* Write config ID */
    op->size = (char)totalSize + 1; /* Bytes to send to FPGA + 1 for checksum */
    op->ctrl = ctrl;                /* 0:i2c 1:Write */
    op->devId = devId;              /* BMI160 Id */
    op->ops = ops;                  /* Number of writes */

    // Initialize payload (zero out memory)
    memset(op->payload, 0, totalSize - sizeof(RamConfig::OperationType));

    return op;
}

//
// TODO :: Refactor / Optimize
//
int RamConfig::Execute() 
{
    ssize_t bytes = 0;
    char ops = 2; /* Set up 2 registers only */

    /* Temporary here before refactor */
    char regSize = ops;
    char dataSize = ops;
    char totalSize = sizeof(RamConfig::OperationType) + regSize + dataSize;

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

    char* dev_0 = dev_0_op->payload;

    dev_0[0] = 0x7E;    /* CMD */
    dev_0[1] = 0x11;    /* Set PMU mode of accelerometer to normal */
    dev_0[2] = 0x40;    /* ACC_CONF */
    dev_0[3] = 0x2C;    /* acc_bwp = 0x2 normal mode + acc_od = 0xC 1600Hz r*/

    dev_0[4] = calculate_checksum((char*)dev_0_op, totalSize); /* totalSize is always 1 less than checksum :: since it was removed from OperationType */

    // [2] Sector
    OperationType* dev_1_op = createOperation(0x53, 0x01, ops);
    if (!dev_1_op) 
    {
        perror("Failed to allocate operation");
        free(dev_0_op);
        return EXIT_FAILURE;
    }

    char* dev_1 = dev_1_op->payload;

    dev_1[0] = 0x2D;    /* CMD */
    dev_1[1] = 0x08;    /* Set PMU mode of accelerometer to normal */
    dev_1[2] = 0x31;    /* ACC_CONF */
    dev_1[3] = 0x00;    /* acc_bwp = 0x2 normal mode + acc_od = 0xC 1600Hz r*/

    dev_1[4] = calculate_checksum((char*)dev_1_op, totalSize); /* totalSize is always 1 less than checksum :: since it was removed from OperationType */

    if(dev_0_op->size > MAX_DMA_TRANSFTER_SIZE)
    {
        fprintf(stderr, "Device 0 operation size exceeds half sector size :: %d bytes\n", dev_0_op->size);
        free(dev_0_op);
        return EXIT_FAILURE;
    }

    if(dev_1_op->size > MAX_DMA_TRANSFTER_SIZE)
    {
        fprintf(stderr, "Device 1 operation size exceeds half sector size :: %d bytes\n", dev_1_op->size);
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

    int i = 0;

    for (i = 0; i < 10; ++i)
    {
        printf("Byte[%zu]: 0x%02x\n", i, ((unsigned char*)dev_0_op)[i]);
    }

    for (i = 0; i < 10; ++i)
    {
        printf("Byte[%zu]: 0x%02x\n", i, ((unsigned char*)dev_1_op)[i]);
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
    printf("[INFO] [RAM] Write %d Bytes to ramDisk to Sector 0\n", bytes);  // Change %ld to %d

    lseek(fd, SECTOR_SIZE * 1, SEEK_SET);

    // Write to sector 1
    bytes = write(fd, dev_0_op, dev_0_op->size);
    if (bytes < 0) 
    {
        perror("Failed to write to sector 1");
        close(fd);
        free(dev_0_op);
        free(dev_1_op);
        return EXIT_FAILURE;
    }
    printf("[INFO] [RAM] Write %d Bytes to ramDisk to Sector 1\n", bytes);  // Change %ld to %d

    lseek(fd, SECTOR_SIZE * 2, SEEK_SET);

    // Write to sector 2
    bytes = write(fd, dev_1_op, dev_1_op->size);
    if (bytes < 0) 
    {
        perror("Failed to write to sector 2");
        close(fd);
        free(dev_0_op);
        free(dev_1_op);
        return EXIT_FAILURE;
    }
    printf("[INFO] [RAM] Write %d Bytes to ramDisk to Sector 2\n", bytes);  // Change %ld to %d

    lseek(fd, SECTOR_SIZE * 3, SEEK_SET);

    // Write to sector 3
    bytes = write(fd, forth_sector, sizeof(forth_sector));
    if (bytes < 0) 
    {
        perror("Failed to write to sector 3");
        close(fd);
        free(dev_0_op);
        free(dev_1_op);
        return EXIT_FAILURE;
    }
    printf("[INFO] [RAM] Write %d Bytes to ramDisk to Sector 3\n", bytes);  // Change %ld to %d

    close(fd);
    free(dev_0_op);
    free(dev_1_op);
    return EXIT_SUCCESS;
}
