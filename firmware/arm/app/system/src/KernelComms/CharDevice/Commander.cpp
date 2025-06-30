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
m_stateChanged(false),
m_calibrationCount(0)
{
    std::cout << "[INFO] [CONSTRUCTOR] " << this << " :: Instantiate Commander" << std::endl;

    setupCommandMatrix();
}

Commander::~Commander()
{
    shutdownThread();
    closeDEV();

    std::cout << "[INFO] [DESTRUCTOR] " << this << " :: Destroy Commander" << std::endl;
}

void Commander::setupCommandMatrix()
{
    m_commandMatrix[CMD_DMA_NORMAL] = {0x04, 0xA1}; /* 0x04A1(ORAL) :: Reconfigure Primary DMA Engine :: Normal Mode 4-Byte */
    m_commandMatrix[CMD_DMA_SENSOR] = {0x5E, 0x50}; /* 0x5E50(SESO) :: Reconfigure Secondary DMA Engine :: Sensor Mode 12-Byte */
    m_commandMatrix[CMD_DMA_SINGLE] = {0x51, 0x6E}; /* 0x516E(SIGE) :: Reconfigure Secondary DMA Engine :: Single Mode 1-Byte */
    m_commandMatrix[CMD_DMA_CUSTOM] = {0xC5, 0x70}; /* 0xC570(CSTO) :: Reconfigure Secondary DMA Engine :: Custom Mode x-Byte */
    m_commandMatrix[CMD_RAMDISK_CONFIG] = {0xC0, 0xF1}; /* 0xC0F1(COFI) :: Activate DMA transfer to send IMU's config to FPGA */
    m_commandMatrix[CMD_RAMDISK_CLEAR]  = {0xC1, 0xEA}; /* 0xC1EA(CLEA) :: Clear DMA variables used for verification of IMU's config */
    m_commandMatrix[CMD_DEBUG_ENABLE]   = {0xDE, 0xBE}; /* 0xDEBE(DEBE) :: Enable debug in kernel space */
    m_commandMatrix[CMD_DEBUG_DISABLE]  = {0xDE, 0xBD}; /* 0xDEBD(DEBD) :: Disable debug in kernel space */
    m_commandMatrix[CMD_FPGA_RESET]     = {0x4E, 0x5E}; /* 0x4E5E(RESE) :: Global Reset to FPGA */
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
    static const std::array<std::string, CMD_AMOUNT> commandStrings =
    {
        "CMD_DMA_NORMAL",
        "CMD_DMA_SENSOR",
        "CMD_DMA_SINGLE",
        "CMD_DMA_CUSTOM",
        "CMD_RAMDISK_CONFIG",
        "CMD_RAMDISK_CLEAR",
        "CMD_DEBUG_ENABLE",
        "CMD_DEBUG_DISABLE",
        "CMD_FPGA_RESET",
    };

    if (cmd >= 0 && cmd < CMD_AMOUNT)
    {
        return commandStrings[cmd];
    }
    else
    {
        return "CMD_UNKNOWN";
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
        /* Write command to Kernel Space */
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

/* THREAD */ void Commander::initThread()
{
    /**
     * Automatically locks the mutex when it is constructed
     * and releases the lock when it goes out of scope
     */
    std::lock_guard<std::mutex> lock(m_threadMutex);

    if (m_CommanderThread.joinable())
    {
        std::cout << "[INFO] [CMD] CommanderThread is already running" << std::endl;
        return;
    }

    std::cout << "[INFO] [CMD] Initialize CommanderThread" << std::endl;

    m_threadKill = false;
    m_CommanderThread = std::thread(&Commander::CommanderThread, this);
}

/* THREAD */ void Commander::shutdownThread()
{
    if (m_file_descriptor >= 0)
    {
        /* Switch to single DMA */
        sendCommand(CMD_DMA_SINGLE);
        /* Reset FPGA */
        sendCommand(CMD_FPGA_RESET);
    }

    /**
     * Automatically locks the mutex when it is constructed
     * and releases the lock when it goes out of scope
     */
    std::lock_guard<std::mutex> lock(m_threadMutex);

    if (m_threadKill)
    {
        std::cout << "[INFO] [CMD] CommanderThread is already marked for shutdown" << std::endl;
        return;
    }

    std::cout << "[INFO] [CMD] Shutdown CommanderThread" << std::endl;

    /**
     * Commander is Event controlled
     * so we must trigger event to
     * escape from IDLE state
     */
    m_threadKill = true;
    triggerCommanderEvent();

    if (m_CommanderThread.joinable())
    {
        m_CommanderThread.join();
        std::cout << "[INFO] [CMD] CommanderThread has been shut down" << std::endl;
    }
}

/* THREAD */ std::string Commander::getIoStateString(ioStateType state)
{
    static const std::array<std::string, IO_AMOUNT> ioStateStrings =
    {
        "IO_COM_IDLE",
        "IO_COM_WRITE",
        "IO_COM_WRITE_ONLY",
        "IO_COM_READ",
        "IO_COM_READ_ONLY",
        "IO_COM_GET_CALIBRATION",
        "IO_COM_SET_CALIBRATION",
    };

    if (state >= 0 && state < IO_AMOUNT)
    {
        return ioStateStrings[state];
    }
    else
    {
        return "UNKNOWN_STATE";
    }
}

/* THREAD */ bool Commander::isThreadKilled()
{
    return m_threadKill;
}

/* THREAD */ void Commander::setCommanderState(ioStateType state)
{
    std::unique_lock<std::mutex> lock(m_ctrlMutex);
    m_ioState = state;
    triggerCommanderEvent();
}

/* THREAD */ void Commander::CommanderThread()
{
    int ret = 0;
    bool check = false;

    while (!m_threadKill)
    {
        if(m_ioState != m_ioStatePrev)
        {
            std::cout << "[INFO] [CMD] State Commander " << m_ioStatePrev << "->" << m_ioState << " " << getIoStateString(m_ioState) << std::endl;
            m_ioStatePrev = m_ioState;
        }

        switch(m_ioState)
        {
            case IO_COM_IDLE:
                waitCommanderEvent(); /* Breaking IDLE on Event */
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
                    m_ioState = IO_COM_IDLE;
                }
                else
                {
                    for (size_t i = 0; i < IO_TRANSFER_SIZE; i++)
                    {
                        (*m_Tx_CommanderVector)[i] = 0x00;
                    }
                    m_ioState = IO_COM_READ;
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
                }
                if(0 < m_calibrationCount)
                {
                    m_ioState = IO_COM_SET_CALIBRATION;
                }
                else
                {
                    m_ioState = IO_COM_IDLE;
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
                }
                else
                {
                    std::cout << "[ERNO] [CMD] Cannot read from kernel space" << std::endl;
                }
                m_ioState = IO_COM_IDLE;
                break;

            case IO_COM_READ_ONLY:

                ret = read(m_file_descriptor, m_Rx_CommanderVector->data(), IO_TRANSFER_SIZE);

                if(ret > 0)
                {
                    std::cout << std::dec << "[INFO] [CMD] Received " << ret << " Bytes of data: ";
                    for (int i = 0; i < ret; ++i)
                    {
                        std::cout << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>((*m_Rx_CommanderVector)[i]) << " ";
                    }
                    std::cout << std::endl;
                }
                else
                {
                    std::cout << "[ERNO] [CMD] Cannot read from kernel space" << std::endl;
                }
                m_ioState = IO_COM_IDLE;
                break;

            case IO_COM_GET_CALIBRATION:

                std::cout << "[ERNO] [CMD] Get Calibration offset" << std::endl;
                ret = read(m_file_descriptor, m_Rx_CommanderVector->data(), IO_TRANSFER_SIZE);

                if (ret == 6)
                {

                    if((*m_Rx_CommanderVector)[1] == 0xDE && (*m_Rx_CommanderVector)[2] == 0xAD && (*m_Rx_CommanderVector)[3] == 0xC0 && (*m_Rx_CommanderVector)[4] == 0xDE)
                    {
                        std::cout << "[ERNO] [CMD] 0xDEADCODE Received -> Going to IO_COM_IDLE" << std::endl;
                        m_ioState = IO_COM_IDLE;
                        break;
                    }
#if 0
                    else
                    {
                        std::cout << std::dec << "[INFO] [CMD] Received " << ret << " Bytes of data: ";
                        for (int i = 0; i < ret; ++i)
                        {
                            std::cout << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>((*m_Rx_CommanderVector)[i]) << " ";
                        }
                        std::cout << std::endl;
                    }
#endif
                    m_x = static_cast<int16_t>(((*m_Rx_CommanderVector)[1] << 8) | (*m_Rx_CommanderVector)[0]);
                    m_y = static_cast<int16_t>(((*m_Rx_CommanderVector)[3] << 8) | (*m_Rx_CommanderVector)[2]);
                    m_z = static_cast<int16_t>(((*m_Rx_CommanderVector)[5] << 8) | (*m_Rx_CommanderVector)[4]);
#if 1
                    setFpgaAverageBuffer(m_x, m_y, m_z);
                    calibrationOfset();
                    m_ioState = IO_COM_IDLE;
#else
                    check = appendBuffer(m_x, m_y, m_z);

                    if(true == check)
                    {
                        averageBuffer();
                        calibrationOfset();
                        m_ioState = IO_COM_IDLE;
                    }
#endif
                }
                else
                {
                    std::cout << "[ERNO] [CMD] Cannot read from kernel space" << std::endl;
                    m_ioState = IO_COM_IDLE;
                }
                break;

            case IO_COM_SET_CALIBRATION:

                offsetType* pOffset;
                pOffset = getCalibrationOfset();
                std::this_thread::sleep_for(std::chrono::milliseconds(100));

                if(0 == m_calibrationCount)
                {
                    std::cout << "[ERNO] [CMD] Set [x] offset" << std::endl;
                    (*m_Tx_CommanderVector)[0] = 0x8A;
                    (*m_Tx_CommanderVector)[1] = 0x11;
                    (*m_Tx_CommanderVector)[2] = 0x71;
                    (*m_Tx_CommanderVector)[3] = pOffset->x;
                    (*m_Tx_CommanderVector)[4] = 0x00;
                    (*m_Tx_CommanderVector)[5] = 0x00;
                    (*m_Tx_CommanderVector)[6] = 0x00;
                    (*m_Tx_CommanderVector)[7] = 0x00;

                    m_calibrationCount = 1;
                    m_ioState = IO_COM_WRITE_ONLY;
                }
                else if(1 == m_calibrationCount)
                {
                    std::cout << "[ERNO] [CMD] Set [y] offset" << std::endl;
                    (*m_Tx_CommanderVector)[0] = 0x8A;
                    (*m_Tx_CommanderVector)[1] = 0x11;
                    (*m_Tx_CommanderVector)[2] = 0x72;
                    (*m_Tx_CommanderVector)[3] = pOffset->y;
                    (*m_Tx_CommanderVector)[4] = 0x00;
                    (*m_Tx_CommanderVector)[5] = 0x00;
                    (*m_Tx_CommanderVector)[6] = 0x00;
                    (*m_Tx_CommanderVector)[7] = 0x00;

                    m_calibrationCount = 2;
                    m_ioState = IO_COM_WRITE_ONLY;
                }
                else if(2 == m_calibrationCount)
                {
                    std::cout << "[ERNO] [CMD] Set [z] offset" << std::endl;
                    (*m_Tx_CommanderVector)[0] = 0x8A;
                    (*m_Tx_CommanderVector)[1] = 0x11;
                    (*m_Tx_CommanderVector)[2] = 0x73;
                    (*m_Tx_CommanderVector)[3] = pOffset->z;
                    (*m_Tx_CommanderVector)[4] = 0x00;
                    (*m_Tx_CommanderVector)[5] = 0x00;
                    (*m_Tx_CommanderVector)[6] = 0x00;
                    (*m_Tx_CommanderVector)[7] = 0x00;

                    m_calibrationCount = 0;;
                    m_ioState = IO_COM_WRITE_ONLY;
                }
                else
                {
                    std::cout << "[ERNO] [CMD] Something wrong with calibration offset" << std::endl;
                    m_calibrationCount = 0;;
                    m_ioState = IO_COM_IDLE;
                }

                break;

            default:
                std::cout << "[INFO] [CMD] Unknown Command" << std::endl;
        }
    }

    std::cout << "[INFO] [CMD] Terminate CommanderThread" << std::endl;
}

/* SHARE */ void Commander::setTransferPointers(
    std::shared_ptr<std::vector<uint8_t>> transferPointerRx,
    std::shared_ptr<std::vector<uint8_t>> transferPointerTx,
    std::shared_ptr<ioStateType> transferState)
{
    if (!transferPointerRx || !transferPointerTx || !transferState)
    {
        std::cerr << "[ERROR] [CMD] One or more transfer pointers are null!" << std::endl;
        return;
    }

    m_Rx_CommanderVector = transferPointerRx;
    m_Tx_CommanderVector = transferPointerTx;
    m_IO_CommanderState = transferState;
}

/* EVENT */ void Commander::waitCommanderEvent()
{
    std::cout << "[INFO] [CMD] CommanderThread Wait" << std::endl;
    std::unique_lock<std::mutex> lock(m_eventMutex);

    auto predicate = [this]()
    {
        return m_stateChanged;
    };

    m_conditionalVariable.wait(lock, predicate);
    m_stateChanged = false;
}

/* EVENT */ void Commander::triggerCommanderEvent()
{
    std::cout << "[INFO] [CMD] CommanderThread Event" << std::endl;
    std::lock_guard<std::mutex> lock(m_eventMutex);

    m_stateChanged = true;
    m_conditionalVariable.notify_one();
}
