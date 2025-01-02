/*!
 * 
 * Author: Ice.Marek
 * IceNET Technology 2024
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
    m_Rx_Commander(new std::vector<char>(CHAR_DEVICE_SIZE)),
    m_Tx_Commander(new std::vector<char>(CHAR_DEVICE_SIZE))
{
    std::cout << "[INFO] [CONSTRUCTOR] " << this << " :: Instantiate Commander" << std::endl;

    /* Initialize m_Rx_Commander and m_Tx_Commander with zeros */
    std::fill(m_Rx_Commander->begin(), m_Rx_Commander->end(), 0);
    std::fill(m_Tx_Commander->begin(), m_Tx_Commander->end(), 0);
}

Commander::~Commander()
{
    std::cout << "[INFO] [DESTRUCTOR] " << this << " :: Destroy Commander" << std::endl;

    closeDEV();

    delete m_Rx_Commander;
    delete m_Tx_Commander;
}

int Commander::openDEV()
{
    m_file_descriptor = open("/dev/KernelCommander", O_RDWR);

    if(m_file_descriptor < 0)
    {
        Error("[CMD] Failed to open Device");
        return ERROR;
    } 
    else 
    {
        Info("[CMD] Device opened successfuly");
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
    int ret = -1;

    Info("[CMD] Command Received :: Sending to Kernel");

    (*m_Tx_Commander)[0] = 0x10;
    (*m_Tx_Commander)[1] = 0xAD;
    ret = write(m_file_descriptor, m_Tx_Commander->data(), 2);

    if (ret == -1)
    {
        Error("[CMD] Cannot write command to kernel space");
        return ERROR;
    }

    m_Tx_Commander->clear();

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
