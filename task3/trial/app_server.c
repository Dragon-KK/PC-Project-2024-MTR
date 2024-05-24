#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>

#define PORT 8800
#define BUFFER_SIZE 1024

int main() 
{
    int server_socket, client_socket;
    socklen_t client_length;
    char buffer[BUFFER_SIZE];
    struct sockaddr_in server_address, client_address;

    // Create socket
    server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket == -1) {
        perror("Error creating socket");
        exit(EXIT_FAILURE);
    }

    // Initialize server address structure
    memset(&server_address, 0, sizeof(server_address));
    server_address.sin_family = AF_INET;
    server_address.sin_addr.s_addr = INADDR_ANY;
    server_address.sin_port = htons(PORT);

    // Bind socket to address and port
    if (bind(server_socket, (struct sockaddr*)&server_address, sizeof(server_address)) == -1) 
    {
        perror("Error binding socket");
        exit(EXIT_FAILURE);
    }

    // Listen for connections
    if (listen(server_socket, 5) == -1) 
    {
        perror("Error listening on socket");
        exit(EXIT_FAILURE);
    }

    printf("Waiting for a connection...\n");

    // Accept incoming connection
    client_length = sizeof(client_address);
    client_socket = accept(server_socket, (struct sockaddr*)&client_address, &client_length);
    if (client_socket == -1) 
    {
        perror("Error accepting connection");
        exit(EXIT_FAILURE);
    }

    printf("Connection accepted\n");

    // Send data to the client
    const char *message = "Hello World!";
    send(client_socket, message, strlen(message), 0);

    // Close sockets
    close(client_socket);
    close(server_socket);

    return 0;
}