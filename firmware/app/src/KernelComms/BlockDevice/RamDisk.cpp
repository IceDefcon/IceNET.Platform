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
#include "RamDisk.h"

RamDisk::RamDisk() :
m_fileDescriptor(-1),
m_instance(this),
m_engineConfig{0}
{
    std::cout << "[INFO] [CONSTRUCTOR] " << m_instance << " :: Instantiate RamDisk" << std::endl;

    ////////////////////////////////////////////////////////////////////////////////
    //
    // OFFLOAD_CTRL :: 8-bits
    //
    ////////////////////////////////////////////////////////////////////////////////
    //
    //  Dma config (Auto/Manual Config)
    //      |
    //      |        Device (I2C, SPI, PWM)
    //      |          ID
    //      |          ||
    //      |          ||
    //      V          VV
    //    | x | xxxx | xx | x | <<<---- OFFLOAD_CTRL : std_logic_vector(6 downto 0)
    //          ΛΛΛΛ        Λ
    //          ||||        |
    //          ||||        |
    //          ||||        |
    //       burst size    R/W (I2C, SPI)
    //       (I2C, SPI)
    //
    ////////////////////////////////////////////////////////////////////////////////

    m_devices =
    {
        {
            0x69, /* BMI160 */
            0x01, /* OFFLOAD_CTRL :: DmaConfig(Auto=0) BurstSize(0) Device(I2C=0) Write(1) */
            {
                {0x7E, 0x11}, /* Soft reset */
                {0x40, 0x2C}  /* Accelerometer config */
            }
        },
        {
            0x53, /* ADXL345 */
            0x01, /* OFFLOAD_CTRL :: DmaConfig(Auto=0) BurstSize(0) Device(I2C=0) Write(1) */
            {
                {0x31, 0x08}, /* Data format */
                {0x2E, 0x08}, /* Interrupt enable */
                {0x2F, 0x00}, /* Interrupt mapping */
                {0x2D, 0x08}, /* Power control */
                {0x2C, 0x0F}  /* Output Data Rate */
            }
        }
    };
}

RamDisk::~RamDisk()
{
    std::cout << "[INFO] [DESTRUCTOR] " << m_instance << " :: Destroy RamDisk" << std::endl;
    if (m_fileDescriptor >= 0)
    {
        closeDEV();
    }

    free(m_BMI160config);
    free(m_ADXL345config);
}

int RamDisk::openDEV()
{
    m_fileDescriptor = open(DEVICE_PATH, O_RDWR);

    if (m_fileDescriptor < 0)
    {
        std::cout << "[ERNO] [RAM] Failed to open Device" << std::endl;
        return EXIT_FAILURE;
    }
    else
    {
        std::cout << "[INFO] [RAM] Device opened successfuly" << std::endl;
    }

    return OK;
}

int RamDisk::dataTX()
{
    /**
     *
     * TODO
     *
     */

    return OK;
}

int RamDisk::dataRX()
{
    return OK; /* One way communication Here */
}

int RamDisk::closeDEV()
{
    if (m_fileDescriptor >= 0)
    {
        close(m_fileDescriptor);
        m_fileDescriptor = -1;
    }

    return OK;
}

char RamDisk::calculateChecksum(const char* data, size_t size)
{
    char checksum = 0;
    for (size_t i = 0; i < size; i++)
    {
        checksum ^= data[i];
    }
    return checksum;
}

DeviceConfig* RamDisk::createOperation(char id, char ctrl, char ops)
{
    char totalSize = sizeof(DeviceConfig) + (2 * ops); /* 2x ---> Regs + Data */

    DeviceConfig* op = (DeviceConfig*)malloc(totalSize);

    if (!op)
    {
        perror("Failed to allocate operation");
        return NULL;
    }

    op->size = (char)totalSize + 1; /* Bytes to send to FPGA + 1 for checksum */
    ////////////////////////////////////////////////////////////////////////////////
    //
    // OFFLOAD_CTRL :: 8-bits
    //
    ////////////////////////////////////////////////////////////////////////////////
    //
    //  Dma config (Auto/Manual Config)
    //      |
    //      |        Device (I2C, SPI, PWM)
    //      |          ID
    //      |          ||
    //      |          ||
    //      V          VV
    //    | x | xxxx | xx | x | <<<---- OFFLOAD_CTRL : std_logic_vector(6 downto 0)
    //          ΛΛΛΛ        Λ
    //          ||||        |
    //          ||||        |
    //          ||||        |
    //       burst size    R/W (I2C, SPI)
    //       (I2C, SPI)
    //
    ////////////////////////////////////////////////////////////////////////////////
    op->ctrl = ctrl;                /* 0:i2c 1:Write */
    op->id = id;                    /* BMI160 Id */
    op->ops = ops;                  /* Number of read/writes */

    memset(op->payload, 0, totalSize - sizeof(DeviceConfig));

    return op;
}

/**
 *
 * Hardcoded configuration
 *
 * Requires knowledge of the devices
 * connected to the FPGA and information
 * about which interface they are connected through
 *
 * In this particular moment we have
 * Two sensors connected over I2C
 * So 3 memory sectors are used
 *
 * [0] Dma Engine Configuration
 * [1] BMI160
 * [2] ADXL345
 *
 */
int RamDisk::assembleConfig()
{
    constexpr uint8_t HEADER_SIZE = 0x04;
    constexpr uint8_t SCRAMBLE_BYTE = 0x77;

    // [0] Sector
    m_engineConfig = (uint8_t*)malloc(HEADER_SIZE);

    if (m_engineConfig == NULL)
    {
        perror("Failed to allocate operation");
    }

    /* TODO :: Need parametrization */
    m_engineConfig[0] = HEADER_SIZE; /* Size of sector 0 */
    m_engineConfig[1] = m_devices.size(); /* Number of Devices to configure */
    m_engineConfig[2] = SCRAMBLE_BYTE; /* Load and Ready */
    m_engineConfig[3] = calculateChecksum((char*)m_engineConfig, 3); /* Only 3 bytes for sub-checksum */

#if 0 /* Test code */
    ////////////////////////////////////////////////////////////////////////
    //
    // TEST :: CODE
    //
    // Iterate over m_devices and create configuration for each
    //
    ////////////////////////////////////////////////////////////////////////
    for (const auto& device : m_devices)
    {
        DeviceConfig* config = createOperation(device.id, device.ctrl, device.registers.size());
        if (!config)
        {
            perror("Failed to allocate device configuration");
            free(m_engineConfig);
            return EXIT_FAILURE;
        }

        // Fill the payload with register-value pairs
        uint8_t* payload = config->payload;
        size_t i = 0;
        for (const auto& reg : device.registers)
        {
            payload[i++] = reg.first;
            payload[i++] = reg.second;
        }

        payload[i] = calculateChecksum((char*)config, sizeof(DeviceConfig) + i);

        m_deviceConfigs.push_back(config);

#if 0 /* Print Device Config Bytes */
        std::cout << "Device ID: " << std::hex << (int)device.id << std::endl; // Print ID as hexadecimal
        uint8_t* bytePtr = reinterpret_cast<uint8_t*>(config);  // Use the current config pointer
        for (size_t i = 0; i < (int)bytePtr[0]; ++i)
        {
            std::cout << "Byte " << i << ": " << std::hex << (int)bytePtr[i] << std::endl;
        }
#endif
    }
#endif

    char BMI160_ops = 2; /* Configure 2 registers only */
    char BMI160_regSize = BMI160_ops;
    char BMI160_dataSize = BMI160_ops;
    char BMI160_totalSize = sizeof(DeviceConfig) + BMI160_regSize + BMI160_dataSize;

    // [1] Sector
    m_BMI160config = createOperation(0x69, 0x01, BMI160_ops);
    if (!m_BMI160config)
    {
        perror("Failed to allocate operation");
        return EXIT_FAILURE;
    }

    uint8_t* BMI160 = m_BMI160config->payload;

    BMI160[0] = 0x7E; /* CMD */
    BMI160[1] = 0x11; /* Set PMU mode of accelerometer to normal */
    BMI160[2] = 0x40; /* ACC_CONF */
    BMI160[3] = 0x2C; /* acc_bwp = 0x2 normal mode + acc_od = 0xC 1600Hz r*/

    BMI160[4] = calculateChecksum((char*)m_BMI160config, BMI160_totalSize); /* totalSize is always 1 less than checksum :: since it was removed from DeviceConfig */

    /* This need parametrization */
    char ADXL_ops = 5;
    char ADXL_regSize = ADXL_ops;
    char ADXL_dataSize = ADXL_ops;
    char ADXL_totalSize = sizeof(DeviceConfig) + ADXL_regSize + ADXL_dataSize;

    // [2] Sector
    m_ADXL345config = createOperation(0x53, 0x01, ADXL_ops);
    if (!m_ADXL345config)
    {
        perror("Failed to allocate operation");
        free(m_BMI160config);
        return EXIT_FAILURE;
    }

    uint8_t* ADXL345 = m_ADXL345config->payload;

    ADXL345[0] = 0x31; /* DATA_FORMAT */
    ADXL345[1] = 0x08; /* Full Resolution | SPI 4 wire | INT_INVERT = active high | FULL_RES = enabled | Justify = unsigned | ±2 g */
    ADXL345[2] = 0x2E; /* INT_ENABLE Register */
    ADXL345[3] = 0x08; /* 0x80 = 1000 0000 (Enable only Data Ready interrupt) */
    ADXL345[4] = 0x2F; /* INT_MAP Register */
    ADXL345[5] = 0x00; /* 0x00 = 0000 0000 (Map Data Ready to INT1)*/
    ADXL345[6] = 0x2D; /* POWER_CTL Register */
    ADXL345[7] = 0x08; /* 0x08 = 0000 1000 (Enable measurement mode) */
    ADXL345[8] = 0x2C; /* BW_RATE Register */
    ADXL345[9] = 0x0F; /* 0x0F = 00001111 (Set ODR to 3200 Hz) */
    ADXL345[10] = calculateChecksum((char*)m_ADXL345config, ADXL_totalSize); /* totalSize is always 1 less than checksum :: since it was removed from DeviceConfig */

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

int RamDisk::sendConfig()
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

    closeDEV();

    return EXIT_SUCCESS;
}

void RamDisk::clearDma()
{
    openDEV();

    const size_t totalSectors = CONFIG_AMOUNT;
    const size_t sectorSize = SECTOR_SIZE;

    char zeroBuffer[SECTOR_SIZE] = {0}; // Buffer filled with zeroes

    for (size_t i = 0; i < totalSectors; i++)
    {
        lseek(m_fileDescriptor, i * sectorSize, SEEK_SET);
        write(m_fileDescriptor, zeroBuffer, sectorSize);
        printf("[INFO] [RAM] Cleared sector %d\n", i);
    }

    closeDEV();
}

