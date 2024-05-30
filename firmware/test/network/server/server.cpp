/*!
 * 
 * Author: Ice.Marek
 * IceNET Technology 2024
 * 
 */
#include <iostream>
#include "tcpServer.h"

int main(int argc, char *argv[]) 
{
    std::cout << "[NET] Initialising TCP Server" << std::endl;

    // Heap allocation
    tcpServer* server = new tcpServer(2555);

    std::cout << "[NET] Listening for a connection..." << std::endl;
    if (!server->initialize()) 
    {
        std::cerr << "[NET] Failed to start server" << std::endl;
        delete server; // clean up
        return 1;
    }

    if (!server->acceptClient()) 
    {
        std::cerr << "[NET] Filed to accept client connection" << std::endl;
        delete server; // clean up
        return 1;
    }

    std::string receivedMessage = server->dataRX(1024);
    if (receivedMessage.empty()) 
    {
        std::cerr << "[NET] Failed to receive message" << std::endl;
        delete server; // clean up
        return 1;
    }

    std::cout << "[NET] Server RX :: " << receivedMessage << std::endl;

    std::string responseMessage("[SERVER] <--- [CLIENT]");
    std::cout << "[NET] Server TX :: " << responseMessage << std::endl;

    if (server->dataTX(responseMessage) < 0) 
    {
        std::cerr << "[NET] Failed to send message" << std::endl;
        delete server; // clean up
        return 1;
    }

    std::cout << "[NET] Transfer complete" << std::endl;

    delete server; // clean up
    return 0;
}
