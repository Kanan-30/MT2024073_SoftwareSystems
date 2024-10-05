#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

#define PORT 5080
#define MAX_CLIENTS 10
#define BUFFER_SIZE 1024
#define DB_FILE "customer_db.txt"

// Function prototypes
void handle_client(int client_socket);
void customer_menu(int client_socket);
void employee_menu(int client_socket);
void manager_menu(int client_socket);
void admin_menu(int client_socket);

int main() {
    int server_socket, client_socket;
    struct sockaddr_in server_address, client_address;
    socklen_t client_addr_len = sizeof(client_address);

    // Create socket
    server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket < 0) {
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }

    // Setup server address
    server_address.sin_family = AF_INET;
    server_address.sin_addr.s_addr = INADDR_ANY;
    server_address.sin_port = htons(PORT);

    // Bind the socket
    if (bind(server_socket, (struct sockaddr *)&server_address, sizeof(server_address)) < 0) {
        perror("Bind failed");
        close(server_socket);
        exit(EXIT_FAILURE);
    }

    // Listen for incoming connections
    listen(server_socket, MAX_CLIENTS);
    printf("Server listening on port %d\n", PORT);

    while (1) {
        // Accept client connection
        client_socket = accept(server_socket, (struct sockaddr *)&client_address, &client_addr_len);
        if (client_socket < 0) {
            perror("Accept failed");
            continue;
        }

        // Create a new process for each client
        pid_t pid = fork();
        if (pid < 0) {
            perror("Fork failed");
            close(client_socket);
            continue;  // In case of error, close socket and continue
        }

        if (pid == 0) {  // Child process
            close(server_socket);  // Close the server socket in the child process
            handle_client(client_socket);
            close(client_socket);  // Close the client socket after serving
            exit(0);  // Terminate child process
        } else {  // Parent process
            close(client_socket);  // Close the client socket in the parent
        }
    }

    close(server_socket);
    return 0;
}

void handle_client(int client_socket) {
    char buffer[BUFFER_SIZE];
    int user_type;

    // Ask for user type
    send(client_socket, "Enter User Type (1: Customer, 2: Employee, 3: Manager, 4: Admin): ", BUFFER_SIZE, 0);
    recv(client_socket, buffer, BUFFER_SIZE, 0);
    user_type = atoi(buffer);

    switch (user_type) {
        case 1:
            customer_menu(client_socket);
            break;
        case 2:
            employee_menu(client_socket);
            break;
        case 3:
            manager_menu(client_socket);
            break;
        case 4:
            admin_menu(client_socket);
            break;
        default:
            send(client_socket, "Invalid User Type!", BUFFER_SIZE, 0);
            break;
    }
}

void customer_menu(int client_socket) {
    int choice;
    char buffer[BUFFER_SIZE];

    do {
        sprintf(buffer, "\nCustomer Menu:\n"
                        "1. Login System\n"
                        "2. Change Password\n"
                        "3. View Account Balance\n"
                        "4. Deposit Money\n"
                        "5. Withdraw Money\n"
                        "6. Transfer Funds\n"
                        "7. Apply for a loan\n"
                        "8. View Transaction History\n"
                        "9. Provide Feedback\n"
                        "10. Logout\n"
                        "11. Exit\n"
                        "Enter your choice: ");

        send(client_socket, buffer, BUFFER_SIZE, 0);
        recv(client_socket, buffer, BUFFER_SIZE, 0);
        choice = atoi(buffer);

        switch (choice) {
            case 1:
                // Customer Login Placeholder
                break;
            case 2: // Change Password
                // Placeholder for implementation
                break;
            case 3: // View Account Balance
                // Placeholder for implementation
                break;
            case 4: // Deposit Money
                // Placeholder for implementation
                break;
            case 5: // Withdraw Money
                // Placeholder for implementation
                break;
            case 6: // Transfer Funds
                // Placeholder for implementation
                break;
            case 7: // Apply for a loan
                // Placeholder for implementation
                break;
            case 8: // View Transaction History
                // Placeholder for implementation
                break;
            case 9: // Provide Feedback
                // Placeholder for implementation
                break;
            case 10: // Logout
                send(client_socket, "Logging out...\n", BUFFER_SIZE, 0);
                return;
            case 11: // Exit
                send(client_socket, "Exiting...\n", BUFFER_SIZE, 0);
                close(client_socket); // Close the socket and exit
                exit(0); // Terminate the child process
                break;
            default:
                send(client_socket, "Invalid choice!\n", BUFFER_SIZE, 0);
                break;
        }
    } while (choice != 10 && choice != 11);
}

void employee_menu(int client_socket) {
    int choice;
    char buffer[BUFFER_SIZE];

    do {
        sprintf(buffer, "\nEmployee Menu:\n"
                        "1. Login System\n"
                        "2. Change Password\n"
                        "3. Add New Customer\n"
                        "4. Process Loan Applications\n"
                        "5. View Assigned Loan Applications\n"
                        "6. Approve/Reject Loans\n"
                        "7. View Customer Transactions\n"
                        "8. Logout\n"
                        "9. Exit\n"
                        "Enter your choice: ");

        send(client_socket, buffer, BUFFER_SIZE, 0);
        recv(client_socket, buffer, BUFFER_SIZE, 0);
        choice = atoi(buffer);

        switch (choice) {
            case 1: // Implement Login System
                // Placeholder for implementation
                break;
            case 2: // Change Password
                // Placeholder for implementation
                break;
            case 3: // Add New Customer
                // Placeholder for implementation
                break;
            case 4: // Process Loan Applications
                // Placeholder for implementation
                break;
            case 5: // View Assigned Loan Applications
                // Placeholder for implementation
                break;
            case 6: // Approve/Reject Loans
                // Placeholder for implementation
                break;
            case 7: // View Customer Transactions
                // Placeholder for implementation
                break;
            case 8: // Logout
                send(client_socket, "Logging out...\n", BUFFER_SIZE, 0);
                return;
            case 9: // Exit
                send(client_socket, "Exiting...\n", BUFFER_SIZE, 0);
                close(client_socket); // Close the socket and exit
                exit(0); // Terminate the child process
                break;
            default:
                send(client_socket, "Invalid choice!\n", BUFFER_SIZE, 0);
                break;
        }
    } while (choice != 8 && choice != 9);
}

void manager_menu(int client_socket) {
    int choice;
    char buffer[BUFFER_SIZE];

    do {
        sprintf(buffer, "\nManager Menu:\n"
                        "1. Login System\n"
                        "2. Activate/Deactivate Customer Accounts\n"
                        "3. Assign Loan Applications to Employees\n"
                        "4. Review Customer Feedback\n"
                        "5. Change Password\n"
                        "6. Logout\n"
                        "7. Exit\n"
                        "Enter your choice: ");

        send(client_socket, buffer, BUFFER_SIZE, 0);
        recv(client_socket, buffer, BUFFER_SIZE, 0);
        choice = atoi(buffer);

        switch (choice) {
            case 1:
                // Placeholder for Login System
                break;
            case 2:
                // Placeholder for Activate/Deactivate Customer Accounts
                break;
            case 3:
                // Placeholder for Assign Loan Applications to Employees
                break;
            case 4:
                // Placeholder for Review Customer Feedback
                break;
            case 5:
                // Placeholder for Change Password
                break;
            case 6:  // Logout
                send(client_socket, "Logging out...\n", BUFFER_SIZE, 0);
                return;  // Return to the main driver menu (do NOT close the connection)
            case 7:  // Exit
                send(client_socket, "Exiting...\n", BUFFER_SIZE, 0);
                close(client_socket);  // Close the socket and terminate the session
                exit(0);  // Terminate the child process
            default:
                send(client_socket, "Invalid choice!\n", BUFFER_SIZE, 0);
                break;
        }
    } while (choice != 7);  // Loop until Exit is selected
}

void admin_menu(int client_socket) {
    int choice;
    char buffer[BUFFER_SIZE];

    do {
        sprintf(buffer, "\nAdmin Menu:\n"
                        "1. Login System\n"
                        "2. Add New Bank Employee\n"
                        "3. Modify Customer Details\n"
                        "4. Modify Employee Details\n"
                        "5. Logout\n"
                        "6. Exit\n"
                        "Enter your choice: ");

        send(client_socket, buffer, BUFFER_SIZE, 0);
        recv(client_socket, buffer, BUFFER_SIZE, 0);
        choice = atoi(buffer);

        switch (choice) {
            case 1:
                // Placeholder for Login System
                break;
            case 2:
                // Placeholder for Add New Bank Employee
                break;
            case 3:
                // Placeholder for Modify Customer Details
                break;
            case 4:
                // Placeholder for Modify Employee Details
                break;
            case 5:  // Logout
                send(client_socket, "Logging out...\n", BUFFER_SIZE, 0);
                return;  // Return to the main driver menu (do NOT close the connection)
            case 6:  // Exit
                send(client_socket, "Exiting...\n", BUFFER_SIZE, 0);
                close(client_socket);  // Close the socket and terminate the session
                exit(0);  // Terminate the child process
            default:
                send(client_socket, "Invalid choice!\n", BUFFER_SIZE, 0);
                break;
        }
    } while (choice != 6);  // Loop until Exit is selected
}

