#include <iostream>
#include "tcp/SocketClient.h"

int main(int argc, char *argv[])
{
    // Check if host address has been given
    if (argc != 2)
    {
        std::cerr << "[ERROR] Missing host address: e.g. ./client 10.0.0.2" << std::endl;
        return 1;
    }

    std::cout << "Initialising TCP Client" << std::endl;

    // Heap allocation
    SocketClient* client = new SocketClient(argv[1], 2555);
    if (client->connectToServer() != 0)
    {
        std::cerr << "[ERROR] Failed to connect to server" << std::endl;
        delete client;
        return 1;
    }

    std::string message("Hello from the Client");
    std::cout << "Sending [" << message << "]" << std::endl;
    if (client->send(message) != 0)
    {
        std::cerr << "[ERROR] Failed to send message" << std::endl;
        delete client;
        return 1;
    }

    std::string receivedMessage = client->receive(1024);
    if (receivedMessage.empty())
    {
        std::cerr << "[ERROR] Failed to receive message" << std::endl;
        delete client;
        return 1;
    }

    std::cout << "Received [" << receivedMessage << "]" << std::endl;
    std::cout << "End of Beagle Board Client Example" << std::endl;

    delete client;
    return 0;
}
