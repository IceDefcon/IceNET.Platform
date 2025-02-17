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
    /**
     *
     * Total Size
     *
     * 4 Bytes ---> DeviceConfig = size + ctrl + id + ops :: Without flex payload[]
     * 2 x ops ---> (Reg + Data) x ops
     * 1 For checksum
     *
     */
    size_t totalSize = sizeof(DeviceConfig) + (2 * ops) + 1;
    DeviceConfig* op = (DeviceConfig*)malloc(totalSize);
    if (!op)
    {
        perror("Failed to allocate operation");
        return NULL;
    }

    // std::cout << "[DEBUG] [ICE] totalSize: " << static_cast<int>(totalSize) << std::endl;

    op->size = static_cast<uint8_t>(totalSize);
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
    op->ctrl = ctrl;  /* 0: I2C, 1: Write */
    op->id = id;      /* BMI160 ID */
    op->ops = ops;    /* Number of read/writes */

    /**
     *
     * Zero the payload
     *
     * Ex. Size = 9(tota) - 4(DeviceConfig) - 1(checksum) = 4
     *
     */
    size_t payloadSize = totalSize - sizeof(DeviceConfig) - 1;
    memset(op->payload, 0, payloadSize);

    // std::cout << "[DEBUG] [ICE] payloadSize: " << static_cast<int>(payloadSize) << std::endl;

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

    for (const auto& device : m_devices)
    {
        DeviceConfig* allocatedConfig = createOperation(device.id, device.ctrl, device.registers.size());

        if (!allocatedConfig)
        {
            perror("Failed to allocate device configuration");
            free(m_engineConfig);
            return EXIT_FAILURE;
        }

        // Fill the payload with register-value pairs
        uint8_t* payload = allocatedConfig->payload;
        size_t i = 0;
        for (const auto& reg : device.registers)
        {
            payload[i++] = reg.first;
            payload[i++] = reg.second;
        }

        payload[i] = calculateChecksum((char*)allocatedConfig, sizeof(DeviceConfig) + i);

        /**
         *
         * Push Back pointers to vector
         * That point to the allocated data
         *
         */
        m_deviceConfigs.push_back(allocatedConfig);
    }

    return EXIT_SUCCESS;
}

int RamDisk::sendConfig()
{
    ssize_t bytes = 0;

    openDEV();

    /* Write to sector 0 */
    lseek(m_fileDescriptor, 0, SEEK_SET);
    bytes = write(m_fileDescriptor, m_engineConfig, HEADER_SIZE);
    if (bytes < 0)
    {
        perror("Failed to write to block device");
        close(m_fileDescriptor);
        free(m_engineConfig);
        return EXIT_FAILURE;
    }
    std::cout << "[INFO] [RAM] Write " << bytes << " Bytes to ramDisk to Sector 0" << std::endl;

    /* Write device configurations */
    for (size_t i = 0; i < m_deviceConfigs.size(); i++)
    {
        lseek(m_fileDescriptor, SECTOR_SIZE * (i + 1), SEEK_SET);
        bytes = write(m_fileDescriptor, m_deviceConfigs[i], m_deviceConfigs[i]->size);
        if (bytes < 0)
        {
            perror("Failed to write to sector");
            close(m_fileDescriptor);
            free(m_engineConfig);

            for (auto& config : m_deviceConfigs)
            {
                free(config);
            }
            m_deviceConfigs.clear();

            return EXIT_FAILURE;
        }
        std::cout << "[INFO] [RAM] Write " << bytes << " Bytes to ramDisk to Sector " << (i + 1) << std::endl;
    }

    closeDEV();

    free(m_engineConfig);
    for (auto& config : m_deviceConfigs)
    {
        free(config);
    }
    m_deviceConfigs.clear();

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

