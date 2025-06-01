/*!
 *
 * Author: Ice.Marek
 * IceNET Technology 2025
 *
 */
#include "KernelCommander.h"

KernelCommander::KernelCommander() :
m_file_descriptor(-1),
m_threadKill(false),
m_ioState(KERNEL_COMMANDER_IDLE),
m_ioStatePrev(KERNEL_COMMANDER_IDLE),
m_Rx_KernelCommanderVector(std::make_shared<std::vector<uint8_t>>(IO_TRANSFER_SIZE)),
m_Tx_KernelCommanderVector(std::make_shared<std::vector<uint8_t>>(IO_TRANSFER_SIZE)),
m_IO_KernelCommanderState(std::make_shared<KernelCommanderStateType>(KERNEL_COMMANDER_IDLE)),
m_commandMatrix(CTRL_CMD_AMOUNT, std::vector<uint8_t>(CMD_LENGTH, 0)),
m_stateChanged(false)
{
    std::cout << "[INFO] [CONSTRUCTOR] " << this << " :: Instantiate KernelCommander" << std::endl;

    setupCommandMatrix();
}

KernelCommander::~KernelCommander()
{
    std::cout << "[INFO] [DESTRUCTOR] " << this << " :: Destroy KernelCommander" << std::endl;

    shutdownThread();
    closeDEV();
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/* CMD */ void KernelCommander::setupCommandMatrix()
{
    m_commandMatrix[CTRL_CMD_NET_ICMP] = {0xD1, 0x1C}; /* 0xD11C(DIIC) :: Scan network mask using ICMP */
    m_commandMatrix[CTRL_CMD_NET_ARP] = {0x1D, 0xAA}; /* 0x1DAA(IDAA) :: Identyfy IPv4 addresses using ARP */
    m_commandMatrix[CTRL_CMD_NET_NDP] = {0x1D, 0xDD}; /* 0x1DDD(IDDD) :: Identyfy IPv6 addresses using NDP */
    m_commandMatrix[CTRL_CMD_NET_TCP] = {0x5E, 0xCC}; /* 0x5ECC(SECC) :: Acivate TCP transmission */
    m_commandMatrix[CTRL_CMD_NET_UDP] = {0x5E, 0xDD}; /* 0x5EDD(SEDD) :: Acivate UDP transmission */
}

/* CMD */ std::string KernelCommander::commandToString(KernelCtrlCommandType cmd)
{
    static const std::array<std::string, CTRL_CMD_AMOUNT> commandStrings =
    {
        "CTRL_CMD_NET_ICMP",
        "CTRL_CMD_NET_ARP",
        "CTRL_CMD_NET_NDP",
        "CTRL_CMD_NET_TCP",
        "CTRL_CMD_NET_UDP",
    };

    if (cmd >= 0 && cmd < CTRL_CMD_AMOUNT)
    {
        return commandStrings[cmd];
    }
    else
    {
        return "CTRL_CMD_UNKNOWN";
    }
}

/* CMD */ int KernelCommander::sendCommand(KernelCtrlCommandType cmd)
{
    int ret = -1;
    std::vector<char>* command = new std::vector<char>(IO_TRANSFER_SIZE);

    std::cout << "[INFO] [ K ] Command Received :: " << commandToString(cmd) << std::endl;

    /* Set command */
    (*command)[0] = m_commandMatrix[cmd][0];
    (*command)[1] = m_commandMatrix[cmd][1];

    /* Write command to Kernel Space */
    ret = write(m_file_descriptor, command->data(), CMD_LENGTH);

    delete command;

    if (ret == -1)
    {
        std::cout << "[ERNO] [ K ] Cannot write command to kernel space" << std::endl;
        return ERROR;
    }

    return OK;
}

/* DEVICE */ int KernelCommander::openDEV()
{
    m_file_descriptor = open("/dev/mainCommander", O_RDWR);

    if(m_file_descriptor < 0)
    {
        std::cout << "[ERNO] [ K ] Failed to open Device" << std::endl;
        m_threadKill = true;
        return ERROR;
    }
    else
    {
        std::cout << "[INFO] [ K ] Device opened successfuly" << std::endl;
        initThread();
    }

    return OK;
}

/* DEVICE */ int KernelCommander::dataRX()
{
    /**
     *
     * TODO
     *
     */
    return OK;
}

/* DEVICE */ int KernelCommander::dataTX()
{
    /**
     *
     * TODO
     *
     */
    return OK;
}

/* DEVICE */ int KernelCommander::closeDEV()
{
    if (m_file_descriptor >= 0)
    {
        close(m_file_descriptor);
        m_file_descriptor = -1; // Mark as closed
    }

    return OK;
}

/* THREAD */ void KernelCommander::initThread()
{
    /**
     * Automatically locks the mutex when it is constructed
     * and releases the lock when it goes out of scope
     */
    std::lock_guard<std::mutex> lock(m_threadMutex);

    if (m_KernelCommanderThread.joinable())
    {
        std::cout << "[INFO] [ K ] KernelCommanderThread is already running" << std::endl;
        return;
    }

    std::cout << "[INFO] [ K ] Initialize KernelCommanderThread" << std::endl;

    m_threadKill = false;
    m_KernelCommanderThread = std::thread(&KernelCommander::KernelCommanderThread, this);
}

/* THREAD */ void KernelCommander::shutdownThread()
{
    if (m_file_descriptor >= 0)
    {
        /**
         *
         * TODO :: Send clean-up commnds
         *
         */
    }

    /**
     * Automatically locks the mutex when it is constructed
     * and releases the lock when it goes out of scope
     */
    std::lock_guard<std::mutex> lock(m_threadMutex);

    if (m_threadKill)
    {
        std::cout << "[INFO] [ K ] KernelCommanderThread is already marked for shutdown" << std::endl;
        return;
    }

    std::cout << "[INFO] [ K ] Shutdown KernelCommanderThread" << std::endl;

    /**
     * KernelCommander is Event controlled
     * so we must trigger event to
     * escape from IDLE state
     */
    m_threadKill = true;
    triggerKernelCommanderEvent();

    if (m_KernelCommanderThread.joinable())
    {
        m_KernelCommanderThread.join();
        std::cout << "[INFO] [ K ] KernelCommanderThread has been shut down" << std::endl;
    }
}

/* THREAD */ std::string KernelCommander::getKernelCommanderStateString(KernelCommanderStateType state)
{
    static const std::array<std::string, KERNEL_COMMANDER_AMOUNT> KernelCommanderStateStrings =
    {
        "KERNEL_COMMANDER_IDLE",
        "KERNEL_COMMANDER_WRITE",
        "KERNEL_COMMANDER_WRITE_ONLY",
        "KERNEL_COMMANDER_READ",
        "KERNEL_COMMANDER_READ_ONLY",
    };

    if (state >= 0 && state < KERNEL_COMMANDER_AMOUNT)
    {
        return KernelCommanderStateStrings[state];
    }
    else
    {
        return "KERNEL_COMMANDER_UNKNOWN_STATE";
    }
}

/* THREAD */ bool KernelCommander::isThreadKilled()
{
    return m_threadKill;
}

/* THREAD */ void KernelCommander::setKernelCommanderState(KernelCommanderStateType state)
{
    std::unique_lock<std::mutex> lock(m_ctrlMutex);
    m_ioState = state;
    triggerKernelCommanderEvent();
}

/* THREAD */ void KernelCommander::printSharedBuffer(std::shared_ptr<std::vector<uint8_t>> buffer)
{
    std::cout << "[INFO] [SHARED] Data in the buffer: ";
    for (size_t i = 0; i < buffer->size(); ++i)
    {
        std::cout << std::hex << std::setw(2) << std::setfill('0') << (int)(*buffer)[i] << " ";
    }
    std::cout << std::endl;
}

/* THREAD */ void KernelCommander::KernelCommanderThread()
{
    int ret = 0;
    bool check = false;

    while (!m_threadKill)
    {
        if(m_ioState != m_ioStatePrev)
        {
            std::cout << "[INFO] [ K ] State KernelCommander " << m_ioStatePrev << "->" << m_ioState << " " << getKernelCommanderStateString(m_ioState) << std::endl;
            m_ioStatePrev = m_ioState;
        }

        switch(m_ioState)
        {
            case KERNEL_COMMANDER_IDLE:
                waitKernelCommanderEvent();
                break;

            case KERNEL_COMMANDER_WRITE:
                std::cout << "[INFO] [ K ] Write to Kernel Commander" << std::endl;
                ret = -1;
                std::cout << "[INFO] [ K ] Data Received :: Sending to Kernel" << std::endl;
                printSharedBuffer(m_Tx_KernelCommanderVector);
                ret = write(m_file_descriptor, m_Tx_KernelCommanderVector->data(), IO_TRANSFER_SIZE);

                if (ret == -1)
                {
                    std::cout << "[ERNO] [ K ] Cannot write command to kernel space" << std::endl;
                    m_ioState = KERNEL_COMMANDER_IDLE;
                }
                else
                {
                    for (size_t i = 0; i < IO_TRANSFER_SIZE; i++)
                    {
                        (*m_Tx_KernelCommanderVector)[i] = 0x00;
                    }
                    m_ioState = KERNEL_COMMANDER_READ;
                }
                break;

            case KERNEL_COMMANDER_WRITE_ONLY:
                std::cout << "[INFO] [ K ] Write to Kernel Commander" << std::endl;
                ret = -1;
                std::cout << "[INFO] [ K ] Data Received :: Sending to Kernel" << std::endl;
                printSharedBuffer(m_Tx_KernelCommanderVector);
                ret = write(m_file_descriptor, m_Tx_KernelCommanderVector->data(), IO_TRANSFER_SIZE);

                if (ret == -1)
                {
                    std::cout << "[ERNO] [ K ] Cannot write command to kernel space" << std::endl;
                }
                else
                {
                    for (size_t i = 0; i < IO_TRANSFER_SIZE; i++)
                    {
                        (*m_Tx_KernelCommanderVector)[i] = 0x00;
                    }
                }
                m_ioState = KERNEL_COMMANDER_IDLE;
                break;

            case KERNEL_COMMANDER_READ:
                std::cout << "[INFO] [ K ] Read from Kernel Commander" << std::endl;
                std::cout << "[INFO] [ K ] Trying to read from Kernel" << std::endl;
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
                ret = read(m_file_descriptor, m_Rx_KernelCommanderVector->data(), IO_TRANSFER_SIZE);

                if(ret > 0)
                {
                    std::cout << "[INFO] [ K ] Received " << ret << " Bytes of data: ";
                    for (int i = 0; i < ret; ++i)
                    {
                        std::cout << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>((*m_Rx_KernelCommanderVector)[i]) << " ";
                    }
                    std::cout << std::endl;
                }
                else
                {
                    std::cout << "[ERNO] [ K ] Cannot read from kernel space" << std::endl;
                }
                m_ioState = KERNEL_COMMANDER_IDLE;
                break;

            case KERNEL_COMMANDER_READ_ONLY:

                ret = read(m_file_descriptor, m_Rx_KernelCommanderVector->data(), IO_TRANSFER_SIZE);

                if(ret > 0)
                {
                    std::cout << std::dec << "[INFO] [ K ] Received " << ret << " Bytes of data: ";
                    for (int i = 0; i < ret; ++i)
                    {
                        std::cout << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>((*m_Rx_KernelCommanderVector)[i]) << " ";
                    }
                    std::cout << std::endl;
                }
                else
                {
                    std::cout << "[ERNO] [ K ] Cannot read from kernel space" << std::endl;
                }
                m_ioState = KERNEL_COMMANDER_IDLE;
                break;



            default:
                std::cout << "[INFO] [ K ] Unknown Command" << std::endl;
        }
    }

    std::cout << "[INFO] [ K ] Terminate KernelCommanderThread" << std::endl;
}

/* SHARE */ void KernelCommander::setTransferPointers(
    std::shared_ptr<std::vector<uint8_t>> transferPointerRx,
    std::shared_ptr<std::vector<uint8_t>> transferPointerTx,
    std::shared_ptr<KernelCommanderStateType> transferState)
{
    if (!transferPointerRx || !transferPointerTx || !transferState)
    {
        std::cerr << "[ERROR] [ K ] One or more transfer pointers are null!" << std::endl;
        return;
    }

    m_Rx_KernelCommanderVector = transferPointerRx;
    m_Tx_KernelCommanderVector = transferPointerTx;
    m_IO_KernelCommanderState = transferState;
}

/* EVENT */ void KernelCommander::waitKernelCommanderEvent()
{
    std::cout << "[INFO] [ K ] KernelCommanderThread Wait" << std::endl;
    std::unique_lock<std::mutex> lock(m_eventMutex);

    auto predicate = [this]()
    {
        return m_stateChanged;
    };

    m_conditionalVariable.wait(lock, predicate);
    m_stateChanged = false;
}

/* EVENT */ void KernelCommander::triggerKernelCommanderEvent()
{
    std::cout << "[INFO] [ K ] KernelCommanderThread Event" << std::endl;
    std::lock_guard<std::mutex> lock(m_eventMutex);

    m_stateChanged = true;
    m_conditionalVariable.notify_one();
}
