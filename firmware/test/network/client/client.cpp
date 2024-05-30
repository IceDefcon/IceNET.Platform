#include <iostream>
#include "tcpClient.h"

int main(int argc, char *argv[])
{
    // Check if host address has been given
    if (argc != 2)
    {
        std::cerr << "[NET] Missing host address: e.g. ./client 10.0.0.2" << std::endl;
        return 1;
    }

    std::cout << "[NET] Initialising TCP Client" << std::endl;

    // Heap allocation
    tcpClient* client = new tcpClient(argv[1], 2555);
    if (client->connectToServer() != 0)
    {
        std::cerr << "[NET] Failed to connect to server" << std::endl;
        delete client;
        return 1;
    }

    std::string message("[CLIENT] ---> [SERVER]");
    std::cout << "[NET] Client TX :: " << message << std::endl;
    if (client->send(message) != 0)
    {
        std::cerr << "[NET] Failed to send message" << std::endl;
        delete client;
        return 1;
    }

    std::string receivedMessage = client->receive(1024);
    if (receivedMessage.empty())
    {
        std::cerr << "[NET] Failed to receive message" << std::endl;
        delete client;
        return 1;
    }

    std::cout << "[NET] Client RX :: " << receivedMessage << std::endl;
    std::cout << "[NET] Shutdown the client" << std::endl;

    delete client;
    return 0;
}
