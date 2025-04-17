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
    m_devices(DEVICE_AMOUNT)
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
    //    | x | xxxx | xx | x | <<<---- OFFLOAD_CTRL : std_logic_vector(7 downto 0)
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
        [DEVICE_BMI160_SPI_1] =
        {
            .id = 0x11, /* Internal SPI Device ID ---> BMI160 :: BUS 0 */
            .ctrl = 0x0B, /* OFFLOAD_CTRL :: DmaConfig(Auto=0) BurstSize(One=0001) Device(SPI=01) Write(1) */
            .registers =
            {
                {0x7E, 0xB6}, /* Soft reset the sensor */
                // {0x7E, 0x03}, /* Start Fast Offset Calibration */
                {0x40, 0x2B}, /* No undersampling, Filter set 2, Output data rate 800Hz */
                {0x41, 0x08}, /* ±8g accelerometer range */
                {0x7E, 0x11}, /* Set accelerometer to normal mode */
                {0x77, 0x40}, /* Enable accel offset only */
                {0x71, 0x00}, /* Calibration x-offset :: (650 / 4096) * 1000 -> 41 = 0x29 */
                {0x72, 0x00}, /* Calibration y-offset :: (260 / 4096) * 1000 ->  23.4mg → 31.7 / 3.9 = 0x08 */
                {0x73, 0x00}, /* Calibration z-offset :: (108 / 4096) * 1000 ->  11.7mg → 11.7 / 3.9 = 0x03 */
                // {0x71, 0x14}, /* Calibration x-offset :: (650 / 8192) * 1000 = 79.3mg → 79.3 / 3.9 = 0x14 */
                // {0x72, 0x08}, /* Calibration y-offset :: (260 / 8192) * 1000 = 23.4mg → 31.7 / 3.9 = 0x08 */
                // {0x73, 0x03}, /* Calibration z-offset :: (108 / 8192) * 1000 = 11.7mg → 11.7 / 3.9 = 0x03 */
                {0x51, 0x10}, /* Enable Data Ready Interrupt */
                {0x56, 0x88}, /* Map Data Ready Interrupt to INT1 and INT2 */
                {0x53, 0xAA}, /* Configure INT1 and INT2 as Outputs + Make them Active High */
                {0x54, 0x00}, /* Both INT1 and INT2 as outputs + non-latched */
            }
        },

        [DEVICE_BMI160_SPI_2] =
        {
            .id = 0x12, /* Internal SPI Device ID ---> BMI160 :: BUS 1 */
            .ctrl = 0x0B, /* OFFLOAD_CTRL :: DmaConfig(Auto=0) BurstSize(One=0001) Device(SPI=01) Write(1) */
            .registers =
            {
                {0x7E, 0xB6}, /* Soft reset the sensor */
                {0x40, 0x28}, /* No undersampling, Filter set 2, Output data rate 1600Hz */
                {0x7E, 0x15}, /* Set gyroscope to normal mode */
                {0x51, 0x10}, /* Enable Data Ready Interrupt */
                {0x56, 0x88}, /* Map Data Ready Interrupt to INT1 and INT2 */
                {0x53, 0xAA}, /* Configure INT1 and INT2 as Outputs + Make them Active High */
                {0x54, 0x00}, /* Both INT1 and INT2 as outputs + non-latched */
            }
        },

        [DEVICE_ADXL345_I2C] =
        {
            .id = 0x53, /* ADXL345 */
            .ctrl = 0x01, /* OFFLOAD_CTRL :: DmaConfig(Auto=0) BurstSize(None=0000) Device(I2C=00) Write(1) */
            .registers =
            {
                {0x31, 0x08}, /* Data format :: Full Resolution */
                {0x2E, 0x80}, /* Interrupt enable :: Data Ready */
                {0x2F, 0x00}, /* Interrupt mapping :: All to INT1 */
                {0x2D, 0x08}, /* Power control :: Measure mode */
                {0x2C, 0x0E}  /* Output Data Rate :: 3200Hz */
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

uint8_t RamDisk::calculateChecksum(const uint8_t* data, size_t size)
{
    uint8_t checksum = 0;
    /* Xor each byte to calculate chacksum */
    for (size_t i = 0; i < size; i++)
    {
        checksum ^= data[i];
    }
    return checksum;
}

DeviceConfigType* RamDisk::createOperation(uint8_t id, uint8_t ctrl, uint8_t ops)
{
    /**
     *
     * Total Size
     *
     * 4 Bytes ---> DeviceConfigType = size + ctrl + id + ops :: Without flex payload[]
     * 2 x ops ---> (Reg + Data) x ops
     * 1 For checksum
     *
     */
    size_t totalSize = sizeof(DeviceConfigType) + (2 * ops) + 1;
    DeviceConfigType* op = (DeviceConfigType*)malloc(totalSize);
    if (!op)
    {
        perror("Failed to allocate operation");
        return nullptr;
    }

    // std::cout << "[INFO] [DEBUG] ID: " << static_cast<int>(id) << " TotalSize: " << static_cast<int>(totalSize) << std::endl;

    op->size = totalSize;
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
    //    | x | xxxx | xx | x | <<<---- OFFLOAD_CTRL : std_logic_vector(7 downto 0)
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
     * Ex. Size = 9(total) - 4(DeviceConfigType) - 1(checksum) = 4
     *
     */
    size_t payloadSize = totalSize - sizeof(DeviceConfigType) - 1;
    memset(op->payload, 0, payloadSize);

    // std::cout << "[INFO] [DEBUG] ID: " << static_cast<int>(id) << " payloadSize: " << static_cast<int>(payloadSize) << std::endl;

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
 * Three sensors connected over SPI and I2C
 * So 4 memory sectors are used
 *
 * [0] Dma Engine Configuration
 * [1] SPI BMI160 S1
 * [2] SPI BMI160 S2
 * [3] I2C ADXL345
 *
 */
int RamDisk::assembleConfig()
{
    /* Sector [0] */
    m_engineConfig.clear();
    /* [0] */ m_engineConfig.push_back(HEADER_SIZE);
    /* [1] */ m_engineConfig.push_back(CONFIGURED_DEVICES);
    /* [2] */ m_engineConfig.push_back(SCRAMBLE_BYTE);
    /* [3] */ m_engineConfig.push_back(calculateChecksum(&m_engineConfig[0], 3));

    std::cout << "[INFO] [RAM] Assemblimg peripheral configuration for " << m_devices.size() << " devices" << std::endl;

    /* Device configuration sectors */
    for (const auto& device : m_devices)
    {
        DeviceConfigType* allocatedConfig = createOperation(device.id, device.ctrl, device.registers.size());

        if (!allocatedConfig)
        {
            perror("Failed to allocate device configuration");
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

        payload[i] = calculateChecksum(reinterpret_cast<uint8_t*>(allocatedConfig), sizeof(DeviceConfigType) + i);

        // Push back the device configuration
        m_deviceConfigs.push_back(allocatedConfig);
    }

    return EXIT_SUCCESS;
}

int RamDisk::sendConfig()
{
    ssize_t bytes = 0;

    openDEV();

    /////////////////////////////////////////////////////////////////////////////////
    //
    // TODO
    //
    // Write to sector 0 (DMA Engine Configuration)
    //
    // Plus all device configuraton in consecutive byte stream
    //
    /////////////////////////////////////////////////////////////////////////////////
    lseek(m_fileDescriptor, 0, SEEK_SET);
    bytes = write(m_fileDescriptor, &m_engineConfig[0], HEADER_SIZE);
    if (bytes < 0)
    {
        perror("Failed to write to block device");
        close(m_fileDescriptor);
        return EXIT_FAILURE;
    }
    std::cout << "[INFO] [RAM] Write " << bytes << " Bytes to ramDisk to Sector 0" << std::endl;

    // Write device configurations to their respective sectors
    for (size_t i = 0; i < m_deviceConfigs.size(); i++)
    {
        /////////////////////////////////////////////////////////////////////////////////
        //
        // TODO :: Move everyting into Sector 0
        //
        // So we dont change the position of the m_fileDescriptor
        //
        // lseek(m_fileDescriptor, SECTOR_SIZE * (i + 1), SEEK_SET);
        //
        /////////////////////////////////////////////////////////////////////////////////
        lseek(m_fileDescriptor, SECTOR_SIZE * (i + 1), SEEK_SET);
        bytes = write(m_fileDescriptor, m_deviceConfigs[i], m_deviceConfigs[i]->size);
        if (bytes < 0)
        {
            perror("Failed to write to sector");
            close(m_fileDescriptor);

            // Free previously allocated device configs
            for (auto& config : m_deviceConfigs)
            {
                free(config);
            }
            m_deviceConfigs.clear();

            return EXIT_FAILURE;
        }
        std::cout << std::dec << "[INFO] [RAM] Write " << bytes << " Bytes to ramDisk to Sector " << (i + 1) << std::endl;
    }

    closeDEV();

    // Free allocated memory for device configurations
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

    const size_t totalSectors = DEVICE_AMOUNT + 1; /* Amount of Densor Devices + DMA Engine Sector */
    const size_t sectorSize = SECTOR_SIZE;
    ssize_t result = 0;

    uint8_t zeroBuffer[SECTOR_SIZE] = {0}; // Buffer filled with zeroes

    for (size_t i = 0; i < totalSectors; i++)
    {
        lseek(m_fileDescriptor, i * sectorSize, SEEK_SET);
        result = write(m_fileDescriptor, zeroBuffer, sectorSize);
        if (result != sectorSize)
        {
            perror("[ERNO] [RAM] Write failed");
        }
        else
        {
            printf("[INFO] [RAM] Cleared sector %zu\n", i);
        }
    }

    closeDEV();
}

