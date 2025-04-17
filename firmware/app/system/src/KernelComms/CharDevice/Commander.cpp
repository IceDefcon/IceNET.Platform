/*!
 * 
 * Author: Ice.Marek
 * IceNET Technology 2025
 * 
 */
#include <chrono> // delay
#include <thread> // delay
#include <iostream>
#include <cstring>
#include <iomanip> // Include the <iomanip> header for setw and setfill
#include <fcntl.h> // For open, O_RDWR, etc.
#include <unistd.h>// For close, read, write, etc.

#include "Commander.h"
#include "Types.h"

Commander::Commander() :
m_file_descriptor(-1),
m_threadKill(false),
m_ioState(IO_COM_IDLE),
m_ioStatePrev(IO_COM_IDLE),
m_Rx_CommanderVector(std::make_shared<std::vector<uint8_t>>(IO_TRANSFER_SIZE)),
m_Tx_CommanderVector(std::make_shared<std::vector<uint8_t>>(IO_TRANSFER_SIZE)),
m_IO_CommanderState(std::make_shared<ioStateType>(IO_COM_IDLE)),
m_commandMatrix(CMD_AMOUNT, std::vector<uint8_t>(CMD_LENGTH, 0)),
m_customDmaSize(0),
m_x(0),
m_y(0),
m_z(0),
m_seconds_since_boot(0)
{
    std::cout << "[INFO] [CONSTRUCTOR] " << this << " :: Instantiate Commander" << std::endl;

    m_commandMatrix[CMD_DMA_NORMAL] = {0x04, 0xA1}; /* 0x04A1(ORAL) :: Reconfigure DMA Engine :: Normal Mode 4-Byte */
    m_commandMatrix[CMD_DMA_SENSOR] = {0x5E, 0x50}; /* 0x5E50(SESO) :: Reconfigure DMA Engine :: Sensor Mode 12-Byte */
    m_commandMatrix[CMD_DMA_SINGLE] = {0x51, 0x6E}; /* 0x516E(SIGE) :: Reconfigure DMA Engine :: Single Mode 1-Byte */
    m_commandMatrix[CMD_DMA_CUSTOM] = {0xC5, 0x70}; /* 0xC570(CSTO) :: Reconfigure DMA Engine :: Custom Mode x-Byte */
    m_commandMatrix[CMD_RAMDISK_CONFIG] = {0xC0, 0xF1}; /* 0xC0F1(COFI) :: Activate DMA transfer to send IMU's config to FPGA */
    m_commandMatrix[CMD_RAMDISK_CLEAR]  = {0xC1, 0xEA}; /* 0xC1EA(CLEA) :: Clear DMA variables used for verification of IMU's config */
    m_commandMatrix[CMD_FPGA_RESET] = {0x4E,0x5E}; /* 0x4E5E(RESE) :: Global Reset to FPGA */
}

Commander::~Commander()
{
    std::cout << "[INFO] [DESTRUCTOR] " << this << " :: Destroy Commander" << std::endl;

    closeDEV();
}

int Commander::openDEV()
{
    m_file_descriptor = open("/dev/KernelCommander", O_RDWR);

    if(m_file_descriptor < 0)
    {
        std::cout << "[ERNO] [CMD] Failed to open Device" << std::endl;
        m_threadKill = true;
        return ERROR;
    } 
    else 
    {
        std::cout << "[INFO] [CMD] Device opened successfuly" << std::endl;
        initThread();
    }

    return OK;
}

int Commander::dataRX()
{
    /**
     * One way communication Here
     *
     * Rx is not Active in fops
     * in the Commander module
     */
    return OK;
}

/**
 *
 * TODO :: Need parametrization
 *
 * Only one command at the call
 * of the dataTx function
 *
 */
int Commander::dataTX()
{
    /**
     *
     * TODO
     *
     */

    return OK;
}

void Commander::setDmaCustom(uint8_t size)
{
    m_customDmaSize = size;
}


std::string Commander::commandToString(commandType cmd)
{
    switch (cmd)
    {
        case CMD_DMA_NORMAL:     return "CMD_DMA_NORMAL";
        case CMD_DMA_SENSOR:     return "CMD_DMA_SENSOR";
        case CMD_DMA_SINGLE:     return "CMD_DMA_SINGLE";
        case CMD_DMA_CUSTOM:     return "CMD_DMA_CUSTOM";
        case CMD_RAMDISK_CONFIG: return "CMD_RAMDISK_CONFIG";
        case CMD_RAMDISK_CLEAR:  return "CMD_RAMDISK_CLEAR";
        case CMD_FPGA_RESET:     return "CMD_FPGA_RESET";
        default:                 return "UNKNOWN_CMD";
    }
}

int Commander::sendCommand(commandType cmd)
{
    int ret = -1;
    std::vector<char>* command = new std::vector<char>(IO_TRANSFER_SIZE);

    std::cout << "[INFO] [CMD] Command Received :: " << commandToString(cmd) << std::endl;

    (*command)[0] = m_commandMatrix[cmd][0];
    (*command)[1] = m_commandMatrix[cmd][1];

    if(CMD_DMA_CUSTOM == cmd)
    {
        (*command)[2] = static_cast<char>(m_customDmaSize);
        ret = write(m_file_descriptor, command->data(), CMD_LENGTH + 1);
    }
    else
    {
        /* Write command to Kernel Space :: To be processed in Kernel */
        ret = write(m_file_descriptor, command->data(), CMD_LENGTH);
    }

    delete command;

    if (ret == -1)
    {
        std::cout << "[ERNO] [CMD] Cannot write command to kernel space" << std::endl;
        return ERROR;
    }

    return OK;
}

int Commander::closeDEV()
{
    if (m_file_descriptor >= 0) 
    {
        close(m_file_descriptor);
        m_file_descriptor = -1; // Mark as closed
    }

    return OK;
}

void Commander::initThread()
{
    /**
     * Automatically locks the mutex when it is constructed
     * and releases the lock when it goes out of scope
     */
    std::lock_guard<std::mutex> lock(m_threadMutex);

    if (m_threadCommander.joinable())
    {
        std::cout << "[INFO] [CMD] threadCommander is already running" << std::endl;
        return;
    }

    std::cout << "[INFO] [CMD] Initialize threadCommander" << std::endl;

    m_threadKill = false;
    m_threadCommander = std::thread(&Commander::threadCommander, this);
}

void Commander::shutdownThread()
{
    /* Switch to single DMA */
    sendCommand(CMD_DMA_SINGLE);
    /* Reset FPGA */
    sendCommand(CMD_FPGA_RESET);

    /**
     * Automatically locks the mutex when it is constructed
     * and releases the lock when it goes out of scope
     */
    std::lock_guard<std::mutex> lock(m_threadMutex);

    if (m_threadKill)
    {
        std::cout << "[INFO] [CMD] threadCommander is already marked for shutdown" << std::endl;
        return;
    }

    std::cout << "[INFO] [CMD] Shutdown threadCommander" << std::endl;

    m_threadKill = true;

    if (m_threadCommander.joinable())
    {
        m_threadCommander.join();
        std::cout << "[INFO] [CMD] threadCommander has been shut down" << std::endl;
    }
}

bool Commander::isThreadKilled()
{
    return m_threadKill;
}

void Commander::threadCommander()
{
    int ret = 0;

    while (!m_threadKill)
    {
        m_ioState = (ioStateType)(*m_IO_CommanderState);

        if(m_ioState != m_ioStatePrev)
        {
            std::cout << "[INFO] [CMD] State Commander " << m_ioStatePrev << "->" << m_ioState << " " << getIoStateString(m_ioState) << std::endl;
            m_ioStatePrev = m_ioState;
        }

        /**
         *
         * INFO
         *
         * Shared Pointer Switch
         *
         * Both state machines change
         * states simultaneously
         * due to share_ptr
         *
         * [1] Commander
         * [2] DroneCtrl
         *
         **/
        switch(*m_IO_CommanderState)
        {
            case IO_COM_IDLE:
                break;

            case IO_COM_WRITE:
                std::cout << "[INFO] [CMD] Write to Kernel Commander" << std::endl;
                ret = -1;
                std::cout << "[INFO] [CMD] Data Received :: Sending to Kernel" << std::endl;
                printSharedBuffer(m_Tx_CommanderVector);
                ret = write(m_file_descriptor, m_Tx_CommanderVector->data(), IO_TRANSFER_SIZE);

                if (ret == -1)
                {
                    std::cout << "[ERNO] [CMD] Cannot write command to kernel space" << std::endl;
                }
                else
                {
                    for (size_t i = 0; i < IO_TRANSFER_SIZE; i++)
                    {
                        (*m_Tx_CommanderVector)[i] = 0x00;
                    }

                    *m_IO_CommanderState = IO_COM_READ;
                }
                break;

            case IO_COM_WRITE_ONLY:
                std::cout << "[INFO] [CMD] Write to Kernel Commander" << std::endl;
                ret = -1;
                std::cout << "[INFO] [CMD] Data Received :: Sending to Kernel" << std::endl;
                printSharedBuffer(m_Tx_CommanderVector);
                ret = write(m_file_descriptor, m_Tx_CommanderVector->data(), IO_TRANSFER_SIZE);

                if (ret == -1)
                {
                    std::cout << "[ERNO] [CMD] Cannot write command to kernel space" << std::endl;
                }
                else
                {
                    for (size_t i = 0; i < IO_TRANSFER_SIZE; i++)
                    {
                        (*m_Tx_CommanderVector)[i] = 0x00;
                    }

                    *m_IO_CommanderState = IO_COM_IDLE;
                }
                break;

            case IO_COM_READ:
                std::cout << "[INFO] [CMD] Read from Kernel Commander" << std::endl;
                std::cout << "[INFO] [CMD] Trying to read from Kernel" << std::endl;
                /**
                 *
                 * TODO
                 *
                 * This is in blocking mode
                 * so timeout have to be introduced
                 * here to unlock kernel space
                 *
                 * Or unlock over the timeout inside kernel space
                 *
                 */
                ret = read(m_file_descriptor, m_Rx_CommanderVector->data(), IO_TRANSFER_SIZE);

                if(ret > 0)
                {
                    std::cout << "[INFO] [CMD] Received " << ret << " Bytes of data: ";
                    for (int i = 0; i < ret; ++i)
                    {
                        std::cout << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>((*m_Rx_CommanderVector)[i]) << " ";
                    }
                    std::cout << std::endl;
                    *m_IO_CommanderState = IO_COM_IDLE;
                }
                else
                {
                    std::cout << "[ERNO] [CMD] Cannot read from kernel space" << std::endl;
                }

                break;

            case IO_COM_READ_ONLY:

                ret = read(m_file_descriptor, m_Rx_CommanderVector->data(), IO_TRANSFER_SIZE);

                // Inside your processing logic
                if (ret == 6)
                {

                    if((*m_Rx_CommanderVector)[1] == 0xDE && (*m_Rx_CommanderVector)[2] == 0xAD && (*m_Rx_CommanderVector)[3] == 0xC0 && (*m_Rx_CommanderVector)[4] == 0xDE)
                    {
                        std::cout << "[ERNO] [CMD] 0xDEADCODE Received -> Going to IO_COM_IDLE" << std::endl;
                        *m_IO_CommanderState = IO_COM_IDLE;
                        break;
                    }

                    m_x = static_cast<int16_t>(((*m_Rx_CommanderVector)[1] << 8) | (*m_Rx_CommanderVector)[0]);
                    m_y = static_cast<int16_t>(((*m_Rx_CommanderVector)[3] << 8) | (*m_Rx_CommanderVector)[2]);
                    m_z = static_cast<int16_t>(((*m_Rx_CommanderVector)[5] << 8) | (*m_Rx_CommanderVector)[4]);

                    m_x_vector[31] = m_x_vector[30];
                    m_x_vector[30] = m_x_vector[29];
                    m_x_vector[29] = m_x_vector[28];
                    m_x_vector[28] = m_x_vector[27];
                    m_x_vector[27] = m_x_vector[26];
                    m_x_vector[26] = m_x_vector[25];
                    m_x_vector[25] = m_x_vector[24];
                    m_x_vector[24] = m_x_vector[23];
                    m_x_vector[23] = m_x_vector[22];
                    m_x_vector[22] = m_x_vector[21];
                    m_x_vector[21] = m_x_vector[20];
                    m_x_vector[20] = m_x_vector[19];
                    m_x_vector[19] = m_x_vector[18];
                    m_x_vector[18] = m_x_vector[17];
                    m_x_vector[17] = m_x_vector[16];
                    m_x_vector[16] = m_x_vector[15];
                    m_x_vector[15] = m_x_vector[14];
                    m_x_vector[14] = m_x_vector[13];
                    m_x_vector[13] = m_x_vector[12];
                    m_x_vector[12] = m_x_vector[11];
                    m_x_vector[11] = m_x_vector[10];
                    m_x_vector[10] = m_x_vector[9];
                    m_x_vector[9] = m_x_vector[8];
                    m_x_vector[8] = m_x_vector[7];
                    m_x_vector[7] = m_x_vector[6];
                    m_x_vector[6] = m_x_vector[5];
                    m_x_vector[5] = m_x_vector[4];
                    m_x_vector[4] = m_x_vector[3];
                    m_x_vector[3] = m_x_vector[2];
                    m_x_vector[2] = m_x_vector[1];
                    m_x_vector[1] = m_x_vector[0];
                    m_x_vector[0] = m_x;

                    m_y_vector[31] = m_y_vector[30];
                    m_y_vector[30] = m_y_vector[29];
                    m_y_vector[29] = m_y_vector[28];
                    m_y_vector[28] = m_y_vector[27];
                    m_y_vector[27] = m_y_vector[26];
                    m_y_vector[26] = m_y_vector[25];
                    m_y_vector[25] = m_y_vector[24];
                    m_y_vector[24] = m_y_vector[23];
                    m_y_vector[23] = m_y_vector[22];
                    m_y_vector[22] = m_y_vector[21];
                    m_y_vector[21] = m_y_vector[20];
                    m_y_vector[20] = m_y_vector[19];
                    m_y_vector[19] = m_y_vector[18];
                    m_y_vector[18] = m_y_vector[17];
                    m_y_vector[17] = m_y_vector[16];
                    m_y_vector[16] = m_y_vector[15];
                    m_y_vector[15] = m_y_vector[14];
                    m_y_vector[14] = m_y_vector[13];
                    m_y_vector[13] = m_y_vector[12];
                    m_y_vector[12] = m_y_vector[11];
                    m_y_vector[11] = m_y_vector[10];
                    m_y_vector[10] = m_y_vector[9];
                    m_y_vector[9] = m_y_vector[8];
                    m_y_vector[8] = m_y_vector[7];
                    m_y_vector[7] = m_y_vector[6];
                    m_y_vector[6] = m_y_vector[5];
                    m_y_vector[5] = m_y_vector[4];
                    m_y_vector[4] = m_y_vector[3];
                    m_y_vector[3] = m_y_vector[2];
                    m_y_vector[2] = m_y_vector[1];
                    m_y_vector[1] = m_y_vector[0];
                    m_y_vector[0] = m_y;

                    m_z_vector[31] = m_z_vector[30];
                    m_z_vector[30] = m_z_vector[29];
                    m_z_vector[29] = m_z_vector[28];
                    m_z_vector[28] = m_z_vector[27];
                    m_z_vector[27] = m_z_vector[26];
                    m_z_vector[26] = m_z_vector[25];
                    m_z_vector[25] = m_z_vector[24];
                    m_z_vector[24] = m_z_vector[23];
                    m_z_vector[23] = m_z_vector[22];
                    m_z_vector[22] = m_z_vector[21];
                    m_z_vector[21] = m_z_vector[20];
                    m_z_vector[20] = m_z_vector[19];
                    m_z_vector[19] = m_z_vector[18];
                    m_z_vector[18] = m_z_vector[17];
                    m_z_vector[17] = m_z_vector[16];
                    m_z_vector[16] = m_z_vector[15];
                    m_z_vector[15] = m_z_vector[14];
                    m_z_vector[14] = m_z_vector[13];
                    m_z_vector[13] = m_z_vector[12];
                    m_z_vector[12] = m_z_vector[11];
                    m_z_vector[11] = m_z_vector[10];
                    m_z_vector[10] = m_z_vector[9];
                    m_z_vector[9] = m_z_vector[8];
                    m_z_vector[8] = m_z_vector[7];
                    m_z_vector[7] = m_z_vector[6];
                    m_z_vector[6] = m_z_vector[5];
                    m_z_vector[5] = m_z_vector[4];
                    m_z_vector[4] = m_z_vector[3];
                    m_z_vector[3] = m_z_vector[2];
                    m_z_vector[2] = m_z_vector[1];
                    m_z_vector[1] = m_z_vector[0];
                    m_z_vector[0] = m_z;

                    m_x_average = (m_x_vector[0] + m_x_vector[1] + m_x_vector[2] + m_x_vector[3] + m_x_vector[4] + m_x_vector[5] + m_x_vector[6] + m_x_vector[7] + m_x_vector[8] + m_x_vector[9] + m_x_vector[10] + m_x_vector[11] + m_x_vector[12] + m_x_vector[13] + m_x_vector[14] + m_x_vector[15] + m_x_vector[16] + m_x_vector[17] + m_x_vector[18] + m_x_vector[19] + m_x_vector[20] + m_x_vector[21] + m_x_vector[22] + m_x_vector[23] + m_x_vector[24] + m_x_vector[25] + m_x_vector[26] + m_x_vector[27] + m_x_vector[28] + m_x_vector[29] + m_x_vector[30] + m_x_vector[31])/32;
                    m_y_average = (m_y_vector[0] + m_y_vector[1] + m_y_vector[2] + m_y_vector[3] + m_y_vector[4] + m_y_vector[5] + m_y_vector[6] + m_y_vector[7] + m_y_vector[8] + m_y_vector[9] + m_y_vector[10] + m_y_vector[11] + m_y_vector[12] + m_y_vector[13] + m_y_vector[14] + m_y_vector[15] + m_y_vector[16] + m_y_vector[17] + m_y_vector[18] + m_y_vector[19] + m_y_vector[20] + m_y_vector[21] + m_y_vector[22] + m_y_vector[23] + m_y_vector[24] + m_y_vector[25] + m_y_vector[26] + m_y_vector[27] + m_y_vector[28] + m_y_vector[29] + m_y_vector[30] + m_y_vector[31])/32;
                    m_z_average = (m_z_vector[0] + m_z_vector[1] + m_z_vector[2] + m_z_vector[3] + m_z_vector[4] + m_z_vector[5] + m_z_vector[6] + m_z_vector[7] + m_z_vector[8] + m_z_vector[9] + m_z_vector[10] + m_z_vector[11] + m_z_vector[12] + m_z_vector[13] + m_z_vector[14] + m_z_vector[15] + m_z_vector[16] + m_z_vector[17] + m_z_vector[18] + m_z_vector[19] + m_z_vector[20] + m_z_vector[21] + m_z_vector[22] + m_z_vector[23] + m_z_vector[24] + m_z_vector[25] + m_z_vector[26] + m_z_vector[27] + m_z_vector[28] + m_z_vector[29] + m_z_vector[30] + m_z_vector[31])/32;

                    clock_gettime(CLOCK_MONOTONIC, &m_ts);
                    m_seconds_since_boot = m_ts.tv_sec + m_ts.tv_nsec / 1e9;

                    std::cout << std::fixed << std::setprecision(6);
                    std::cout << "[INFO] [CMD] [" << m_seconds_since_boot << "] " << std::dec
                              << "Average Acceleration [" << m_x_average << "," << m_y_average << "," << m_z_average << "]" << std::endl;
                }
                else
                {
                    std::cout << "[ERNO] [CMD] Cannot read from kernel space" << std::endl;
                }

                break;

            default:
                std::cout << "[INFO] [CMD] Unknown Command" << std::endl;
        }

        /* Reduce consumption of CPU resources */
        // std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }

    std::cout << "[INFO] [CMD] Terminate threadCommander" << std::endl;
}

/* SHARE */ void Commander::setTransferPointers(
std::shared_ptr<std::vector<uint8_t>> transferPointerRx,
std::shared_ptr<std::vector<uint8_t>> transferPointerTx,
std::shared_ptr<ioStateType> transferState)
{
    m_Rx_CommanderVector = transferPointerRx;
    m_Tx_CommanderVector = transferPointerTx;
    m_IO_CommanderState = transferState;
}
