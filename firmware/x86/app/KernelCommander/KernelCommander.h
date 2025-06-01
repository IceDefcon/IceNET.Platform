/*!
 *
 * Author: Ice.Marek
 * IceNET Technology 2025
 *
 */
#pragma once

#include <condition_variable>
#include <unistd.h>
#include <iostream>
#include <cstring>
#include <iomanip>
#include <fcntl.h>
#include <chrono>
#include <thread>
#include <string>
#include <atomic>
#include <vector>
#include <mutex>
#include <array>

const size_t CMD_LENGTH = 2;
const size_t IO_TRANSFER_SIZE = 8;

typedef enum
{
    CTRL_CMD_NET_ICMP,
    CTRL_CMD_NET_ARP,
    CTRL_CMD_NET_NDP,
    CTRL_CMD_NET_TCP,
    CTRL_CMD_NET_UDP,
    CTRL_CMD_AMOUNT,
} KernelCtrlCommandType;

typedef enum
{
    OK,
    ERROR,
    UNKNOWN
} ProcessStatusType;

typedef enum
{
    KERNEL_COMMANDER_IDLE,
    KERNEL_COMMANDER_WRITE,
    KERNEL_COMMANDER_WRITE_ONLY,
    KERNEL_COMMANDER_READ,
    KERNEL_COMMANDER_READ_ONLY,
    KERNEL_COMMANDER_AMOUNT,
}KernelCommanderStateType;

class KernelCommander
{
    private:
        int m_file_descriptor;

        /* Thread Variables */
        std::thread m_KernelCommanderThread;
        std::mutex m_threadMutex;
        std::mutex m_ctrlMutex;
        bool m_threadKill;

        KernelCommanderStateType m_ioState;
        KernelCommanderStateType m_ioStatePrev;

        std::shared_ptr<std::vector<uint8_t>> m_Rx_KernelCommanderVector;
        std::shared_ptr<std::vector<uint8_t>> m_Tx_KernelCommanderVector;
        std::shared_ptr<KernelCommanderStateType> m_IO_KernelCommanderState;

        std::vector<std::vector<uint8_t>> m_commandMatrix;

        /* Event Variables */
        std::mutex m_eventMutex;
        std::condition_variable m_conditionalVariable;
        bool m_stateChanged;

    public:
        KernelCommander();
        ~KernelCommander();

        /* CMD */ void setupCommandMatrix();
        /* CMD */ std::string commandToString(KernelCtrlCommandType cmd);
        /* CMD */ int sendCommand(KernelCtrlCommandType cmd);

        /* DEVICE */ int openDEV();
        /* DEVICE */ int dataRX();
        /* DEVICE */ int dataTX();
        /* DEVICE */ int closeDEV();

        /* THREAD */ void initThread();
        /* THREAD */ void shutdownThread();
        /* THREAD */ std::string getKernelCommanderStateString(KernelCommanderStateType state);
        /* THREAD */ bool isThreadKilled();
        /* THREAD */ void setKernelCommanderState(KernelCommanderStateType state);
        /* THREAD */ void printSharedBuffer(std::shared_ptr<std::vector<uint8_t>> buffer);
        /* THREAD */ void KernelCommanderThread();

        /* SHARE */ void setTransferPointers(
        std::shared_ptr<std::vector<uint8_t>> transferPointerRx,
        std::shared_ptr<std::vector<uint8_t>> transferPointerTx,
        std::shared_ptr<KernelCommanderStateType> transferState);

        /* EVENT */ void waitKernelCommanderEvent();
        /* EVENT */ void triggerKernelCommanderEvent();
};
