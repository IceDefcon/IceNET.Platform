#include <iostream>
#include <cstring>
#include <chrono>
#include <thread>
#include <linux/can.h>
#include <linux/can/raw.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <net/if.h>
#include <unistd.h>

int main() {
    std::cout << "Debug 0 " << std::endl;

    // Open CAN0 socket
    int can0Socket = socket(PF_CAN, SOCK_RAW, CAN_RAW);
    if (can0Socket < 0) {
        std::cerr << "Failed to open CAN0 socket." << std::endl;
        return 1;
    }
    std::cout << "Debug 1 " << std::endl;

    // Set interface name
    struct ifreq ifr;
    std::strcpy(ifr.ifr_name, "can0");
    ioctl(can0Socket, SIOCGIFINDEX, &ifr);
    std::cout << "Debug 2 " << std::endl;

    // Bind socket to the interface
    struct sockaddr_can can0Addr;
    can0Addr.can_family = AF_CAN;
    can0Addr.can_ifindex = ifr.ifr_ifindex;
    if (bind(can0Socket, reinterpret_cast<struct sockaddr*>(&can0Addr), sizeof(can0Addr)) < 0) {
        std::cerr << "Failed to bind CAN0 socket." << std::endl;
        close(can0Socket);
        return 1;
    }
    std::cout << "Debug 3 " << std::endl;

    // Open CAN1 socket
    int can1Socket = socket(PF_CAN, SOCK_RAW, CAN_RAW);
    if (can1Socket < 0) {
        std::cerr << "Failed to open CAN1 socket." << std::endl;
        close(can0Socket);
        return 1;
    }
    std::cout << "Debug 4 " << std::endl;

    // Set interface name
    std::strcpy(ifr.ifr_name, "can1");
    ioctl(can1Socket, SIOCGIFINDEX, &ifr);
    std::cout << "Debug 5 " << std::endl;

    // Bind socket to the interface
    struct sockaddr_can can1Addr;
    can1Addr.can_family = AF_CAN;
    can1Addr.can_ifindex = ifr.ifr_ifindex;
    if (bind(can1Socket, reinterpret_cast<struct sockaddr*>(&can1Addr), sizeof(can1Addr)) < 0) {
        std::cerr << "Failed to bind CAN1 socket." << std::endl;
        close(can0Socket);
        close(can1Socket);
        return 1;
    }
    std::cout << "Debug 5a " << std::endl;

    // Prepare CAN frames
    struct can_frame canFrame;
    canFrame.can_id = 0x123; // Example CAN identifier
    canFrame.can_dlc = 8;    // Example data length
    std::cout << "Debug 6 " << std::endl;

    // Loop and send data from CAN0 to CAN1
    while (true) {
        // Fill the data bytes with some pattern
        for (int i = 0; i < canFrame.can_dlc; ++i) {
            canFrame.data[i] = i;
        }
    std::cout << "Debug ba " << std::endl;

        // Send the frame on CAN0
        if (write(can0Socket, &canFrame, sizeof(canFrame)) < 0) {
            std::cerr << "Failed to send data on CAN0." << std::endl;
        }
    std::cout << "Debug 6b " << std::endl;

        // Receive the frame on CAN1
        if (read(can1Socket, &canFrame, sizeof(canFrame)) < 0) {
            std::cerr << "Failed to receive data on CAN1." << std::endl;
        }

    std::cout << "Debug 6c " << std::endl;

        // Delay before the next iteration
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
    std::cout << "Debug 7 " << std::endl;

    // Close the sockets
    close(can0Socket);
    close(can1Socket);

    return 0;
}