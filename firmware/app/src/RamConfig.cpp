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
    op->ops = ops;                  /* Number of read/writes */

    memset(op->payload, 0, totalSize - sizeof(DeviceConfig));

    return op;
}

int RamConfig::AssembleData()
{
    /* TODO :: Need parametrization */
    m_engineConfig[0] = 0x02;
    m_engineConfig[1] = 0x11;
    m_engineConfig[2] = 0x02;
    m_engineConfig[3] = 0x13;

    char ops = 2; /* Set up 2 registers only */

    /* Temporary here before refactor */
    char regSize = ops;
    char dataSize = ops;
    char totalSize = sizeof(DeviceConfig) + regSize + dataSize;

    // [1] Sector
    m_BMI160config = createOperation(0x69, 0x01, ops);
    if (!m_BMI160config)
    {
        perror("Failed to allocate operation");
        return EXIT_FAILURE;
    }

    uint8_t* BMI160 = m_BMI160config->payload;

    BMI160[0] = 0x7E;    /* CMD */
    BMI160[1] = 0x11;    /* Set PMU mode of accelerometer to normal */
    BMI160[2] = 0x40;    /* ACC_CONF */
    BMI160[3] = 0x2C;    /* acc_bwp = 0x2 normal mode + acc_od = 0xC 1600Hz r*/

    BMI160[4] = calculate_checksum((char*)m_BMI160config, totalSize); /* totalSize is always 1 less than checksum :: since it was removed from DeviceConfig */

    // [2] Sector
    m_ADXL345config = createOperation(0x53, 0x01, ops);
    if (!m_ADXL345config)
    {
        perror("Failed to allocate operation");
        free(m_BMI160config);
        return EXIT_FAILURE;
    }

    uint8_t* ADXL345 = m_ADXL345config->payload;

    ADXL345[0] = 0x2D;    /* CMD */
    ADXL345[1] = 0x08;    /* Set PMU mode of accelerometer to normal */
    ADXL345[2] = 0x31;    /* ACC_CONF */
    ADXL345[3] = 0x00;    /* acc_bwp = 0x2 normal mode + acc_od = 0xC 1600Hz r*/

    ADXL345[4] = calculate_checksum((char*)m_ADXL345config, totalSize); /* totalSize is always 1 less than checksum :: since it was removed from DeviceConfig */

    if(m_BMI160config->size > MAX_DMA_TRANSFER_SIZE)
    {
        fprintf(stderr, "Device 0 operation size exceeds half sector size :: %d bytes\n", m_BMI160config->size);
        free(m_BMI160config);
        return EXIT_FAILURE;
    }

    if(m_ADXL345config->size > MAX_DMA_TRANSFER_SIZE)
    {
        fprintf(stderr, "Device 1 operation size exceeds half sector size :: %d bytes\n", m_ADXL345config->size);
        free(m_ADXL345config);
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}

int RamConfig::openDEV()
{
    int ret = EXIT_FAILURE;
    m_fileDescriptor = open(DEVICE_PATH, O_RDWR);

    if (m_fileDescriptor < 0)
    {
        Error("[RAM] Failed to open Device");
        return EXIT_FAILURE;
    }
    else
    {
        Info("[RAM] Device opened successfuly");
        ret = AssembleData();

        if(ret == EXIT_SUCCESS)
        {
            Info("[RAM] Data Assembled Successfully");
        }
        else
        {
            Error("[RAM] Failed to Assembly data");
        }
    }

    return OK;
}

int RamConfig::dataRX()
{
    return OK; /* One way communication Here */
}

int RamConfig::launchEngine()
{
    ssize_t bytes = 0;

    openDEV();

    /* Write to sector 0 */
    lseek(m_fileDescriptor, 0, SEEK_SET);
    bytes = write(m_fileDescriptor, m_engineConfig, sizeof(m_engineConfig));
    if (bytes < 0)
    {
        perror("Failed to write to block device");
        close(m_fileDescriptor);
        free(m_BMI160config);
        free(m_ADXL345config);
        return EXIT_FAILURE;
    }
    printf("[INFO] [RAM] Write %d Bytes to ramDisk to Sector 0\n", bytes);

    closeDEV();

    return EXIT_SUCCESS;
}


int RamConfig::dataTX()
{
    ssize_t bytes = 0;

    openDEV();

    /* Write to sector 1 */
    lseek(m_fileDescriptor, SECTOR_SIZE * 1, SEEK_SET);
    bytes = write(m_fileDescriptor, m_BMI160config, m_BMI160config->size);
    if (bytes < 0)
    {
        perror("Failed to write to sector 1");
        close(m_fileDescriptor);
        free(m_BMI160config);
        free(m_ADXL345config);
        return EXIT_FAILURE;
    }
    printf("[INFO] [RAM] Write %d Bytes to ramDisk to Sector 1\n", bytes);


    /* Write to sector 2 */
    lseek(m_fileDescriptor, SECTOR_SIZE * 2, SEEK_SET);
    bytes = write(m_fileDescriptor, m_ADXL345config, m_ADXL345config->size);
    if (bytes < 0)
    {
        perror("Failed to write to sector 2");
        close(m_fileDescriptor);
        free(m_BMI160config);
        free(m_ADXL345config);
        return EXIT_FAILURE;
    }
    printf("[INFO] [RAM] Write %d Bytes to ramDisk to Sector 2\n", bytes);

    free(m_BMI160config);
    free(m_ADXL345config);

    closeDEV();

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
