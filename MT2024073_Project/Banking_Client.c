#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

#define SERVER_IP "127.0.0.1"
#define PORT 5080
#define BUFFER_SIZE 1024

int main() {
    int sock;
    struct sockaddr_in server_address;
    char buffer[BUFFER_SIZE];

    while (1) {
        // Create socket
        sock = socket(AF_INET, SOCK_STREAM, 0);
        if (sock < 0) {
            perror("Socket creation failed");
            exit(EXIT_FAILURE);
        }

        // Setup server address
        server_address.sin_family = AF_INET;
        server_address.sin_port = htons(PORT);
        inet_pton(AF_INET, SERVER_IP, &server_address.sin_addr);

        // Connect to the server
        if (connect(sock, (struct sockaddr *)&server_address, sizeof(server_address)) < 0) {
            perror("Connection failed");
            close(sock);
            exit(EXIT_FAILURE);
        }

        while (1) {
            memset(buffer, 0, BUFFER_SIZE);
            recv(sock, buffer, BUFFER_SIZE, 0);
            printf("%s", buffer);
            
            // Get user input
            fgets(buffer, BUFFER_SIZE, stdin);
            send(sock, buffer, strlen(buffer), 0);
            
            // Check for exit condition
            if (strncmp(buffer, "11", 2) == 0) { // Check if Exit is chosen
                printf("Exiting client...\n");
                close(sock);
                return 0; // Exit the client
            }

            if (strncmp(buffer, "10", 2) == 0) { // Check if Logout is chosen
                printf("Logging out...\n");
                close(sock);
                break; // Break out to re-establish the connection for re-login
            }
        }
    }

    return 0;
}

