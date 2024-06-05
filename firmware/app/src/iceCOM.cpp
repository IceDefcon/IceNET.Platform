/*!
 * 
 * Author: Ice.Marek and Ventor
 * IceNET Technology 2024
 * 
 */
#include <iostream> 		// IO devices :: keyboard
#include <fcntl.h> 			// Open device
#include <string.h> 		// strlem
#include <unistd.h> 		// read/write to the file
#include <cstring> 			// strcmp
#include <termios.h> 		// terminal settings
#include <cstdint>          // for uint8_t
#include <chrono>           // sleep_for
#include <thread>           // sleep_for
#include <vector>
#include <iomanip> // for std::hex and std::setfill


#include "iceCOM.h"

#define iceDEV "/dev/iceCOM"

iceCOM::iceCOM(): 
m_file_descriptor(0), 
m_killThread(false),
m_charDeviceRx(CHAR_DEVICE_SIZE),
m_charDeviceTx(CHAR_DEVICE_SIZE),
m_consoleControl(CHAR_CONSOLE_SIZE),
m_charRxReady(false),
m_charTxReady(false)
{
    /* Initialize m_charDeviceRx, m_charDeviceTx, and m_consoleControl with zeros */
    std::fill(m_charDeviceRx.begin(), m_charDeviceRx.end(), 0);
    std::fill(m_charDeviceTx.begin(), m_charDeviceTx.end(), 0);
    std::fill(m_consoleControl.begin(), m_consoleControl.end(), 0);

    Info("[CONSTRUCTOR] Initialise iceCOM Object");
}

iceCOM::~iceCOM() 
{
	Info("[DESTRUCTOR] Destroy iceCOM Object");
    if (m_iceCOMThread.joinable()) 
    {
    	m_iceCOMThread.join();
   	}
}

int iceCOM::openCOM() 
{
    m_file_descriptor = open(iceDEV, O_RDWR);

    if(m_file_descriptor < 0)
    {
        Error("[COM] Failed to open Device");
        m_killThread = true;
        return ERROR;
    } 
    else 
    {
        Info("[COM] Device opened successfuly");
        initThread();
    }

    return OK;
}

int iceCOM::dataRX()
{
    int ret;

    // Attempt to read data from kernel space
    ret = read(m_file_descriptor, m_charDeviceRx.data(), CHAR_DEVICE_SIZE);
    if (ret == -1)
    {
        Error("[COM] Cannot read from kernel space");
        return ERROR;
    }
    else if (ret == 0)
    {
        Error("[COM] No data available");
        return ENODATA;
    }
    else
    {
        // Print received data for debugging
        Read(m_charDeviceRx.data());

        /* Clear char device Rx buffer */
        m_charDeviceRx.clear();

        return OK;
    }
}

int iceCOM::dataTX()
{
    int ret = -1;

    Write();
#if 0 /* Disable console input */
    /* Get console characters */
    std::cin.getline(m_consoleControl.data(), CHAR_CONSOLE_SIZE);

    if (std::strcmp(m_consoleControl.data(), "exit") == 0) 
    {
        m_killThread = true;
        return ret;
    }
    /* Read Enable in FIFO */
    else if (std::strcmp(m_consoleControl.data(), "rd") == 0)
    {
        m_charDeviceTx[0] = 0x12; /* Custom Kernel Byte Map :: Check reciprocal in charDevice.c */
        m_charDeviceTx[1] = 0x34; /* Custom Kernel Byte Map :: Check reciprocal in charDevice.c */
        ret = write(m_file_descriptor, m_charDeviceTx.data(), 2);
        return ret;
    }
#endif
#if 1 /* Data received from TCP-->SM-->CHAR */

    while (true) 
    {
        if (true == getCharTxReady()) 
        {
            Info("[COM] TCP ---> SM");
            setCharTxReady(false);
            break;
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }

    std::cout << "[INFO] [COM] Hex values of m_charDeviceTx: ";
    for (int i = 0; i < 4 && i < m_charDeviceTx.size(); ++i) 
    {
        std::cout << "0x" << std::hex << std::setw(2) << std::setfill('0')
                  << static_cast<int>(m_charDeviceTx[i]) << " ";
    }
    std::cout << std::endl;

#else
    /**
     * 
     * We have to pass data trough the FIFO
     * to separate bytes from eachother 
     * in order to pass them to the 
     * i2c state machine in FPGA
     * 
     * Byte[0] :: Device Address
     * Byte[1] :: Register Address
     * Byte[2] :: Register Control
     * Byte[3] :: Register Data
     * 
     */
    m_charDeviceTx[0] = Compute::computeDeviceAddress(m_consoleControl.data());
    m_charDeviceTx[1] = Compute::computeRegisterAddress(m_consoleControl.data());
    m_charDeviceTx[2] = Compute::computeRegisterControl(m_consoleControl.data());

    /* If Write then compute RegisterData */
    if(m_charDeviceTx[2] == 0x01)
    {
        m_charDeviceTx[3] = Compute::computeRegisterData(m_consoleControl.data());

        if(m_charDeviceTx[0] == 0xFF || m_charDeviceTx[1] == 0xFF || m_charDeviceTx[2] == 0xFF || m_charDeviceTx[3] == 0xFF) 
        {
            Error("[COM] Bytes computation failure [WR]");
            return ret;
        }
    }
    else
    {
        if(m_charDeviceTx[0] == 0xFF || m_charDeviceTx[1] == 0xFF || m_charDeviceTx[2] == 0xFF) 
        {
            Error("[COM] Bytes computation failure [RD]");
            return ret;
        }
        
        /**
         * 
         * Additional byte 
         * at read procedure
         * to always make 4 bytes 
         * FIFO input/output geometry
         * 
         */
        m_charDeviceTx[3] = 0x00;
    }
#endif

    ret = write(m_file_descriptor, m_charDeviceTx.data(), 4);

    if (ret == -1)
    {
        Error("[COM] Cannot write to kernel space");
        return ERROR;
    }

    std::this_thread::sleep_for(std::chrono::seconds(1));
    m_charDeviceTx[0] = 0x12; /* Custom Kernel Byte Map :: Check reciprocal in charDevice.c */
    m_charDeviceTx[1] = 0x34; /* Custom Kernel Byte Map :: Check reciprocal in charDevice.c */
    ret = write(m_file_descriptor, m_charDeviceTx.data(), 2);

    /* Clear charDevice Rx buffer */
    m_charDeviceTx.clear();
    /* Clear console control buffer */
    m_consoleControl.clear();

    return OK;
}

int iceCOM::closeCOM() 
{
    if (m_file_descriptor >= 0) 
    {
        close(m_file_descriptor);
        m_file_descriptor = -1; // Mark as closed
    }

    return OK;
}

void iceCOM::initThread()
{
    Info("[COM] Init the iceCOMThread");
    m_iceCOMThread = std::thread(&iceCOM::iceCOMThread, this);
}

bool iceCOM::isThreadKilled()
{
	return m_killThread;
}

void iceCOM::iceCOMThread()
{
    Info("[COM] Enter iceCOMThread");

    while(!m_killThread) 
    {
        if(OK != dataTX())
        {
            Error("[COM] Cannot write into the console");
        }
        else
        {
            /**
             * 
             * At the moment Feedback only print info 
             * about successfully transfered command 
             * 
             */
            if(OK != dataRX())
            {
                Error("[COM] Cannot read from the console");
            }

            /* TODO :: Set the flag to indicate the data is ready */

        }

        /* Reduce consumption of CPU resources */
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }

    Info("[COM] Terminate iceCOMThread");
}

/**
 * 
 * Data
 * 
 */
std::vector<char>* iceCOM::getCharDeviceRx()
{
    return &m_charDeviceRx;
}

void iceCOM::setCharDeviceTx(std::vector<char>* charVector)
{
    m_charDeviceTx = *charVector;
}

/**
 * 
 * Flags
 * 
 */
bool iceCOM::getCharRxReady()
{
    return m_charRxReady;
}

void iceCOM::setCharRxReady(bool flag)
{
    m_charRxReady = flag;
}

bool iceCOM::getCharTxReady()
{
    return m_charTxReady;
}

void iceCOM::setCharTxReady(bool flag)
{
    m_charTxReady = flag;
}