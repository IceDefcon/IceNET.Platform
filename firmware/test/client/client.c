#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define SERVER_PORT 12345
#define SERVER_IP "10.0.0.2" // Assuming the server is running locally

int main() 
{
    int client_socket;
    struct sockaddr_in server_addr;
    char buffer[1024] = {0};
    char *message = "Hello from client";

    // Create socket
    if ((client_socket = socket(AF_INET, SOCK_STREAM, 0)) < 0) 
    {
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }
    else
    {
        printf("Socket created successfuly\n");
    }

    // Set server address structure
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(SERVER_PORT);
    if (inet_pton(AF_INET, SERVER_IP, &server_addr.sin_addr) <= 0) 
    {
        perror("Invalid address/ Address not supported");
        exit(EXIT_FAILURE);
    }
    else
    {
        printf("Valid address detected\n");
    }

    // Connect to server
    if (connect(client_socket, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) 
    {
        perror("Connection failed");
        exit(EXIT_FAILURE);
    }
    else
    {
        printf("Connected to server\n");
    }

    // Send message to server
    send(client_socket, message, strlen(message), 0);
    printf("Message sent to server: %s\n", message);

#if 0
    // Receive message from server
    recv(client_socket, buffer, sizeof(buffer), 0);
    printf("Message from server: %s\n", buffer);
#endif
    // Close socket
    close(client_socket);

    return 0;
}
