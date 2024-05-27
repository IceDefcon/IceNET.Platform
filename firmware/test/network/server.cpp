#include <iostream>
#include "network/SocketServer.h"

int main(int argc, char *argv[])
{
    std::cout << "Initialising TCP Server" << std::endl;

    // Heap allocation
    SocketServer* server = new SocketServer(2555);

    std::cout << "Listening for a connection..." << std::endl;
    if (server->listen() != 0) 
    {
        std::cerr << "Error: failed to start server" << std::endl;
        delete server; // clean up
        return 1;
    }

    std::string receivedMessage = server->receive(1024);
    if (receivedMessage.empty()) 
    {
        std::cerr << "Error: failed to receive message" << std::endl;
        delete server; // clean up
        return 1;
    }

    std::cout << "Received from the client [" << receivedMessage << "]" << std::endl;

    std::string responseMessage("The Server says thanks!");
    std::cout << "Sending back [" << responseMessage << "]" << std::endl;

    if (server->send(responseMessage) != 0) 
    {
        std::cerr << "Error: failed to send message" << std::endl;
        delete server; // clean up
        return 1;
    }

    std::cout << "End of Beagle Board Server Example" << std::endl;

    delete server; // clean up
    return 0;
}
