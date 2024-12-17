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


RamConfig::RamConfig() :
m_fileDescriptor(0)
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

DeviceConfig* RamConfig::createOperation(char id, char ctrl, char ops)
{
    char totalSize = sizeof(DeviceConfig) + (2 * ops); /* 2x ---> Regs + Data */

    DeviceConfig* op = (DeviceConfig*)malloc(totalSize);

    if (!op)
    {
        perror("Failed to allocate operation");
        return NULL;
    }

    op->size = (char)totalSize + 1; /* Bytes to send to FPGA + 1 for checksum */
    op->ctrl = ctrl;                /* 0:i2c 1:Write */
    op->id = id;                    /* BMI160 Id */
    op->ops = ops;                  /* Number of writes */

    memset(op->payload, 0, totalSize - sizeof(DeviceConfig));

    return op;
}

int RamConfig::AssembleData()
{
    m_firstSector[0] = 0x02;

    m_fourthSector[0] = 0x00;
    m_fourthSector[1] = 0x69;
    m_fourthSector[2] = 0x00;
    m_fourthSector[3] = 0x00;

    return OK;
}

int RamConfig::openDEV()
{
    m_fileDescriptor = open(DEVICE_PATH, O_RDWR);

    if (m_fileDescriptor < 0)
    {
        Error("[RAM] Failed to open Device");
        return EXIT_FAILURE;
    }
    else
    {
        Info("[RAM] Device opened successfuly");
    }

    return OK;
}

int RamConfig::dataRX()
{
    return OK; /* One way communication Here */
}

int RamConfig::dataTX()
{
    AssembleData();

    ssize_t bytes = 0;
    char ops = 2; /* Set up 2 registers only */

    /* Temporary here before refactor */
    char regSize = ops;
    char dataSize = ops;
    char totalSize = sizeof(DeviceConfig) + regSize + dataSize;

    // [1] Sector
    DeviceConfig* dev_0_op = createOperation(0x69, 0x01, ops);
    if (!dev_0_op)
    {
        perror("Failed to allocate operation");
        return EXIT_FAILURE;
    }

    uint8_t* dev_0 = dev_0_op->payload;

    dev_0[0] = 0x7E;    /* CMD */
    dev_0[1] = 0x11;    /* Set PMU mode of accelerometer to normal */
    dev_0[2] = 0x40;    /* ACC_CONF */
    dev_0[3] = 0x2C;    /* acc_bwp = 0x2 normal mode + acc_od = 0xC 1600Hz r*/

    dev_0[4] = calculate_checksum((char*)dev_0_op, totalSize); /* totalSize is always 1 less than checksum :: since it was removed from DeviceConfig */

    // [2] Sector
    DeviceConfig* dev_1_op = createOperation(0x53, 0x01, ops);
    if (!dev_1_op)
    {
        perror("Failed to allocate operation");
        free(dev_0_op);
        return EXIT_FAILURE;
    }

    uint8_t* dev_1 = dev_1_op->payload;

    dev_1[0] = 0x2D;    /* CMD */
    dev_1[1] = 0x08;    /* Set PMU mode of accelerometer to normal */
    dev_1[2] = 0x31;    /* ACC_CONF */
    dev_1[3] = 0x00;    /* acc_bwp = 0x2 normal mode + acc_od = 0xC 1600Hz r*/

    dev_1[4] = calculate_checksum((char*)dev_1_op, totalSize); /* totalSize is always 1 less than checksum :: since it was removed from DeviceConfig */

    if(dev_0_op->size > MAX_DMA_TRANSFER_SIZE)
    {
        fprintf(stderr, "Device 0 operation size exceeds half sector size :: %d bytes\n", dev_0_op->size);
        free(dev_0_op);
        return EXIT_FAILURE;
    }

    if(dev_1_op->size > MAX_DMA_TRANSFER_SIZE)
    {
        fprintf(stderr, "Device 1 operation size exceeds half sector size :: %d bytes\n", dev_1_op->size);
        free(dev_1_op);
        return EXIT_FAILURE;
    }

    //
    // Write to sector 0
    //
    bytes = write(m_fileDescriptor, m_firstSector, sizeof(m_firstSector));
    if (bytes < 0)
    {
        perror("Failed to write to block device");
        close(m_fileDescriptor);
        free(dev_0_op);
        free(dev_1_op);
        return EXIT_FAILURE;
    }
    printf("[INFO] [RAM] Write %d Bytes to ramDisk to Sector 0\n", bytes);  // Change %ld to %d

    lseek(m_fileDescriptor, SECTOR_SIZE * 1, SEEK_SET);

    //
    // Write to sector 1
    //
    bytes = write(m_fileDescriptor, dev_0_op, dev_0_op->size);
    if (bytes < 0)
    {
        perror("Failed to write to sector 1");
        close(m_fileDescriptor);
        free(dev_0_op);
        free(dev_1_op);
        return EXIT_FAILURE;
    }
    printf("[INFO] [RAM] Write %d Bytes to ramDisk to Sector 1\n", bytes);  // Change %ld to %d

    lseek(m_fileDescriptor, SECTOR_SIZE * 2, SEEK_SET);

    //
    // Write to sector 2
    //
    bytes = write(m_fileDescriptor, dev_1_op, dev_1_op->size);
    if (bytes < 0)
    {
        perror("Failed to write to sector 2");
        close(m_fileDescriptor);
        free(dev_0_op);
        free(dev_1_op);
        return EXIT_FAILURE;
    }
    printf("[INFO] [RAM] Write %d Bytes to ramDisk to Sector 2\n", bytes);  // Change %ld to %d

    lseek(m_fileDescriptor, SECTOR_SIZE * 3, SEEK_SET);

    //
    // Write to sector 3
    //
    bytes = write(m_fileDescriptor, m_fourthSector, sizeof(m_fourthSector));
    if (bytes < 0)
    {
        perror("Failed to write to sector 3");
        close(m_fileDescriptor);
        free(dev_0_op);
        free(dev_1_op);
        return EXIT_FAILURE;
    }
    printf("[INFO] [RAM] Write %d Bytes to ramDisk to Sector 3\n", bytes);  // Change %ld to %d

    close(m_fileDescriptor);
    free(dev_0_op);
    free(dev_1_op);
    return EXIT_SUCCESS;
}

int RamConfig::closeDEV()
{
    if (m_fileDescriptor >= 0)
    {
        close(m_fileDescriptor);
        m_fileDescriptor = 0; // Mark as closed
    }

    return OK;
}
