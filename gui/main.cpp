#include <iostream>
#include <cstring>
#include <arpa/inet.h>
#include <unistd.h>

#define PORT 12345
#define SERVER_IP "10.0.0.2"  // IP address of the BeagleBone Black

int main() {
    int sock = 0;
    struct sockaddr_in serv_addr;
    char send_buffer[] = {0xAB};  // Send 0xAB
    char recv_buffer[1];

    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        std::cerr << "Socket creation error" << std::endl;
        return -1;
    }

    memset(&serv_addr, '0', sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(PORT);

    if (inet_pton(AF_INET, SERVER_IP, &serv_addr.sin_addr) <= 0) {
        std::cerr << "Invalid address/ Address not supported" << std::endl;
        return -1;
    }

    if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
        std::cerr << "Connection Failed" << std::endl;
        return -1;
    }

    send(sock, send_buffer, sizeof(send_buffer), 0);
    std::cout << "Data sent: 0xAB" << std::endl;

    recv(sock, recv_buffer, sizeof(recv_buffer), 0);
    std::cout << "Received data: 0x" << std::hex << (unsigned int)recv_buffer[0] << std::endl;

    close(sock);
    return 0;
}
