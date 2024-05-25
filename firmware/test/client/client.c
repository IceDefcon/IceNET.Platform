#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>

#define MAX_MESSAGE_SIZE 2000

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
    if(socket_desc == -1){
        perror("Unable to create socket");
        return -1;
    }
    printf("Socket created successfully\n");
    
    // Set port and IP the same as server-side:
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(2005);
    server_addr.sin_addr.s_addr = inet_addr("10.0.0.2");
    
    // Send connection request to server:
    printf("Attempting to connect to server...\n");
    if(connect(socket_desc, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0){
        perror("Unable to connect");
        return -1;
    }
    printf("Connected with server successfully\n");
    
    // Get input from the user:
    printf("Enter message: ");
    fgets(client_message, sizeof(client_message), stdin);
    if (strlen(client_message) >= MAX_MESSAGE_SIZE) {
        fprintf(stderr, "Message is too long\n");
        return -1;
    }
    
    // Send the message to server:
    printf("Sending message to server...\n");
    if(send(socket_desc, client_message, strlen(client_message), 0) < 0){
        perror("Unable to send message");
        return -1;
    }
    printf("Message sent successfully\n");
    
    // Receive the server's response:
    printf("Waiting for server response...\n");
    // ssize_t bytes_received = recv(socket_desc, server_message, sizeof(server_message), 0);
    // if(bytes_received < 0){
    //     perror("Error while receiving server's msg");
    //     return -1;
    // }
    // server_message[bytes_received] = '\0'; // Null-terminate the received message
    // printf("Server's response: %s\n", server_message);
    
    // Close the socket:
    close(socket_desc);
    printf("Socket closed\n");
    
    return 0;
}
