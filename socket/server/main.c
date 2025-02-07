// Server side C program to demonstrate Socket
// programming
#include <pthread.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

#define PORT 8080

// Function for handling each client in a new thread
void *handle_connection(void *socket_desc) {
    int new_socket = *(int *)socket_desc;
    free(socket_desc); // Free memory allocated for socket descriptor

    char buffer[1024] = {0};
    ssize_t valread = read(new_socket, buffer, sizeof(buffer) - 1);
    if (valread > 0) {
        buffer[valread] = '\0'; // Ensure null termination
        printf("Client: %s\n", buffer);
    }

    char *response = "Hello from server";
    send(new_socket, response, strlen(response), 0);
    printf("Response sent to client.\n");

    close(new_socket); // Close client socket
    return NULL;
}

int main() {
    int server_fd, *new_socket;
    struct sockaddr_in address;
    int opt = 1;
    socklen_t addrlen = sizeof(address);

    // Create socket
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        perror("Socket failed");
        exit(EXIT_FAILURE);
    }

    // Set socket options
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt))) {
        perror("Setsockopt failed");
        exit(EXIT_FAILURE);
    }

    // Configure address structure
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);

    // Bind socket
    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
        perror("Bind failed");
        exit(EXIT_FAILURE);
    }

    // Listen for incoming connections
    if (listen(server_fd, 10) < 0) { // 10 = backlog (max pending connections)
        perror("Listen failed");
        exit(EXIT_FAILURE);
    }

    printf("Server listening on port %d...\n", PORT);

    // Accept incoming connections in a loop
    while (1) {
        new_socket = malloc(sizeof(int)); // Allocate memory for the new socket
        if (!new_socket) {
            perror("Memory allocation failed");
            continue;
        }

        *new_socket = accept(server_fd, (struct sockaddr *)&address, &addrlen);
        if (*new_socket < 0) {
            perror("Accept failed");
            free(new_socket);
            continue;
        }

        printf("New connection accepted.\n");

        pthread_t thread;
        if (pthread_create(&thread, NULL, handle_connection, (void *)new_socket) != 0) {
            perror("Failed to create thread");
            free(new_socket);
        }

        pthread_detach(thread);	// Automatically clean up thread resources when finished
	
    }

    // Close server socket (never reached in this infinite loop)
    close(server_fd);
    return 0;
}

