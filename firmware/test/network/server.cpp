#include <iostream>
#include "tcp/tcpServer.h"

int main(int argc, char *argv[]) 
{
    std::cout << "Initialising TCP Server" << std::endl;

    // Heap allocation
    tcpServer* server = new tcpServer(2555);

    std::cout << "Listening for a connection..." << std::endl;
    if (!server->initialize()) 
    {
        std::cerr << "Error: failed to start server" << std::endl;
        delete server; // clean up
        return 1;
    }

    if (!server->acceptClient()) 
    {
        std::cerr << "Error: failed to accept client connection" << std::endl;
        delete server; // clean up
        return 1;
    }

    std::string receivedMessage = server->dataRX(1024);
    if (receivedMessage.empty()) 
    {
        std::cerr << "Error: failed to receive message" << std::endl;
        delete server; // clean up
        return 1;
    }

    std::cout << "Received from the client [" << receivedMessage << "]" << std::endl;

    std::string responseMessage("The Server says thanks!");
    std::cout << "Sending back [" << responseMessage << "]" << std::endl;

    if (server->dataTX(responseMessage) < 0) 
    {
        std::cerr << "Error: failed to send message" << std::endl;
        delete server; // clean up
        return 1;
    }

    std::cout << "End of Server Example" << std::endl;

    delete server; // clean up
    return 0;
}
