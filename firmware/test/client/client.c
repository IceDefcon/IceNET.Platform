#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>

#define MAX_MESSAGE_SIZE 2000
#define SERVER_PORT 2555
#define SERVER_IP "10.0.0.2"

int main(void)
{
    int socket_desc;
    struct sockaddr_in server_addr;
    char server_message[MAX_MESSAGE_SIZE], client_message[MAX_MESSAGE_SIZE];
    
    // Clean buffers:
    memset(server_message, '\0', sizeof(server_message));
    memset(client_message, '\0', sizeof(client_message));
    
    // Create socket:
    socket_desc = socket(AF_INET, SOCK_STREAM, 0);
    if(socket_desc == -1)
    {
        perror("Unable to create socket");
        return -1;
    }
    printf("Socket created successfully\n");
    
    // Set port and IP the same as server-side:
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(SERVER_PORT);
    server_addr.sin_addr.s_addr = inet_addr(SERVER_IP);
    
    // Send connection request to server:
    printf("Attempting to connect to server...\n");
    if(connect(socket_desc, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0)
    {
        perror("Unable to connect");
        return -1;
    }
    printf("Connected with server successfully\n");
    
    // Get input from the user:
    printf("Enter message: ");
    if (fgets(client_message, sizeof(client_message), stdin) == NULL)
    {
        perror("Error reading input");
        return -1;
    }
    
    // Remove newline character if present
    size_t message_len = strlen(client_message);
    if (client_message[message_len - 1] == '\n') 
    {
        client_message[message_len - 1] = '\0';
        message_len--;
    }
    
    // Send the message to server:
    printf("Sending message to server...\n");
    ssize_t total_sent = 0;
    while (total_sent < message_len) 
    {
        ssize_t sent = send(socket_desc, client_message + total_sent, message_len - total_sent, 0);
        if (sent < 0)
        {
            perror("Unable to send message");
            return -1;
        }
        total_sent += sent;
    }
    printf("Message sent successfully\n");

#if 0 /* TODO :: Need synchronisation */
    // Receive the server's response:
    printf("Waiting for server response...\n");
    ssize_t bytes_received = recv(socket_desc, server_message, sizeof(server_message) - 1, 0);
    if(bytes_received < 0)
    {
        perror("Error while receiving server's msg");
        return -1;
    }
    server_message[bytes_received] = '\0'; // Null-terminate the received message
    printf("Server's response: %s\n", server_message);
#endif

    // Close the socket:
    close(socket_desc);
    printf("Socket closed\n");
    
    return 0;
}
