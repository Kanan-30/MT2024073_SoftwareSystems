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
    int choice;

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
            // Main role selection menu
            memset(buffer, 0, BUFFER_SIZE);
            recv(sock, buffer, BUFFER_SIZE, 0); // Receive the main driver menu (role selection)
            printf("%s", buffer);  // Display the main menu to the client

            // Get the user's role selection (Customer, Employee, Manager, Admin)
            fgets(buffer, BUFFER_SIZE, stdin);
            send(sock, buffer, strlen(buffer), 0); // Send the selected role to the server

            int logged_out = 0;  // Track whether the user logged out to re-display the main menu

            // Inside the selected role menu loop (Customer, Employee, Manager, Admin)
            while (1) {
                memset(buffer, 0, BUFFER_SIZE);
                recv(sock, buffer, BUFFER_SIZE, 0);  // Receive the specific role menu (e.g., Customer menu)
                printf("%s", buffer);  // Display the specific role menu to the client

                // Get user input for the menu option
                fgets(buffer, BUFFER_SIZE, stdin);
                send(sock, buffer, strlen(buffer), 0);  // Send the selected option to the server

                choice = atoi(buffer);  // Convert user input to integer

                // Check if the choice is Logout or Exit
                if (choice == 10 || choice == 8 || choice == 5 || choice == 4) {  // Logout for different menus
                    printf("Logging out...\n");
                    logged_out = 1;
                    break;  // Break from the role-specific menu loop, but remain in the outer loop for role selection
                }

                if (choice == 11 || choice == 9 || choice == 6 || choice == 5) {  // Exit for different menus
                    printf("Exiting client...\n");
                    close(sock);  // Close the socket and exit
                    return 0;  // Terminate the client program
                }
            }

            // If logged out, break from the main loop to re-display the main driver menu
            if (logged_out) {
                break;  // Break from the role-specific menu and go back to role selection
            }
        }

        // Close the socket only if exiting the entire application
        close(sock);
    }

    return 0;
}

