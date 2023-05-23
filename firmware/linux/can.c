#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <net/if.h>
#include <linux/can.h>
#include <linux/can/raw.h>

int main() {
    int can0Socket;
    struct sockaddr_can canAddr;
    struct ifreq ifr;
    struct can_frame canFrame;
    
    printf("Debug 1\n");
    // Create socket
    can0Socket = socket(PF_CAN, SOCK_RAW, CAN_RAW);
    if (can0Socket < 0) {
        perror("Failed to open CAN socket");
        return 1;
    }
    printf("Debug 2\n");
    
    // Set interface name
    strcpy(ifr.ifr_name, "can0");
    ioctl(can0Socket, SIOCGIFINDEX, &ifr);
    printf("Debug 3\n");
    
    // Bind socket to the interface
    canAddr.can_family = AF_CAN;
    canAddr.can_ifindex = ifr.ifr_ifindex;
    if (bind(can0Socket, (struct sockaddr *)&canAddr, sizeof(canAddr)) < 0) {
        perror("Failed to bind CAN socket");
        close(can0Socket);
        return 1;
    }
    printf("Debug 4\n");
    
    // Prepare CAN frame
    canFrame.can_id = 0x123; // Example CAN identifier
    canFrame.can_dlc = 8;    // Example data length
    memset(canFrame.data, 0x55, canFrame.can_dlc); // Example data
    printf("Debug 5\n");
    
    // Send CAN frame
    if (write(can0Socket, &canFrame, sizeof(canFrame)) < 0) {
        perror("Failed to send CAN frame");
        close(can0Socket);
        return 1;
    }
    printf("Debug 6\n");
    
    // Close the socket
    close(can0Socket);
    
    return 0;
}
