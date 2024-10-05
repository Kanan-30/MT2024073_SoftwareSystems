#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <time.h>
#define PORT 5080
#define MAX_CLIENTS 10
#define BUFFER_SIZE 1024
#define CUSTOMER_DB "customer_db.txt"
#define EMPLOYEE_DB "employee_db.txt"
#define MANAGER_DB "manager_db.txt"
#define ADMIN_DB "admin_db.txt"
#define BUFFER_SIZE 1024


char logged_in_username[50];  // Global variable to track logged-in user

// Function prototypes
void handle_client(int client_socket);
void customer_menu(int client_socket);
void employee_menu(int client_socket);
void manager_menu(int client_socket);
void admin_menu(int client_socket);
void save_customer_to_file(const char *username, const char *password, int amount);
int login_user(int client_socket, const char *role_file, const char *role);
int create_new_account(int client_socket, const char *role_file, const char *role);
void generate_customer_id(char *customer_id);

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
            if (login_user(client_socket, CUSTOMER_DB, "Customer")) {
                customer_menu(client_socket);
            }
            break;
        case 2:
            if (login_user(client_socket, EMPLOYEE_DB, "Employee")) {
                employee_menu(client_socket);
            }
            break;
        case 3:
            if (login_user(client_socket, MANAGER_DB, "Manager")) {
                manager_menu(client_socket);
            }
            break;
        case 4:
            if (login_user(client_socket, ADMIN_DB, "Admin")) {
                admin_menu(client_socket);
            }
            break;
        default:
            send(client_socket, "Invalid User Type!", BUFFER_SIZE, 0);
            break;
    }
}

// Function to trim newline and spaces from the buffer
void trim_newline(char *str) {
    int len = strlen(str);
    if (len > 0 && str[len-1] == '\n') {
        str[len-1] = '\0';  // Remove newline character
    }
}

// Function to login user from a file
int login_user(int client_socket, const char *role_file, const char *role) {
    char buffer[BUFFER_SIZE], username[50], password[50], file_username[50], file_password[50];
    char line[BUFFER_SIZE];
    FILE *file;

    // Ask for username
    send(client_socket, "Enter Username: ", BUFFER_SIZE, 0);
    recv(client_socket, username, sizeof(username), 0);
    trim_newline(username);  // Clean input

    // Open the role file (e.g., customer, employee file)
    file = fopen(role_file, "r");
    if (file == NULL) {
        perror("Error opening file");
        return 0;
    }

    int found = 0;
    // Read each line and check if the username exists
    while (fgets(line, sizeof(line), file)) {
        sscanf(line, "%s %s", file_username, file_password);  // Split line into username and password
        if (strcmp(file_username, username) == 0) {
            found = 1;  // Username found
            break;
        }
    }
    fclose(file);

    if (found) {
        // Ask for password if username exists
        send(client_socket, "Enter Password: ", BUFFER_SIZE, 0);
        recv(client_socket, password, sizeof(password), 0);
        trim_newline(password);  // Clean input

        if (strcmp(file_password, password) == 0) {
            send(client_socket, "Login successful!\n", BUFFER_SIZE, 0);
            strcpy(logged_in_username, username);
            return 1;  // Successful login
        } else {
            send(client_socket, "Login failed! Incorrect password.\n", BUFFER_SIZE, 0);
            return 0;  // Password mismatch
        }
    } else {
        // If username not found, prompt to create a new account
        send(client_socket, "Username not found! Create a new account? (yes/no): ", BUFFER_SIZE, 0);
        recv(client_socket, buffer, sizeof(buffer), 0);
        trim_newline(buffer);  // Clean input

        if (strcmp(buffer, "yes") == 0) {
            return create_new_account(client_socket, role_file, role);  // Create account if yes
        } else {
            send(client_socket, "Login cancelled.\n", BUFFER_SIZE, 0);
            return 0;  // Cancel if no
        }
    }
}

// Function to create a new account
int create_new_account(int client_socket, const char *role_file, const char *role) {
    char username[50], password[50];
    FILE *file = fopen(role_file, "a");  // Open file in append mode

    if (file == NULL) {
        perror("Error opening file");
        return 0;
    }

    // Get the new username and password
    send(client_socket, "Enter new Username: ", BUFFER_SIZE, 0);
    recv(client_socket, username, sizeof(username), 0);
    trim_newline(username);  // Clean input

    send(client_socket, "Enter new Password: ", BUFFER_SIZE, 0);
    recv(client_socket, password, sizeof(password), 0);
    trim_newline(password);  // Clean input

    // Save the new user to the file (username password)
    fprintf(file, "%s %s\n", username, password);
    fclose(file);

    send(client_socket, "Account created successfully!\n", BUFFER_SIZE, 0);
    return 1;
}
void generate_customer_id(char *customer_id) {
    srand(time(NULL));
    sprintf(customer_id, "CUST%05d", rand() % 100000);
}

void customer_menu(int client_socket) {
    int choice;
    char buffer[BUFFER_SIZE];

    // Login or register the customer before presenting menu options
  //  customer_login(client_socket);

    do {
        sprintf(buffer, "\nCustomer Menu:\n"
                        "1. Change Password\n"
                        "2. View Account Balance\n"
                        "3. Deposit Money\n"
                        "4. Withdraw Money\n"
                        "5. Transfer Funds\n"
                        "6. Apply for a loan\n"
                        "7. View Transaction History\n"
                        "8. Provide Feedback\n"
                        " ____Enter logout to logout____\n"
			" ____Enter exit to exit    ____\n"

                        "Enter your choice: ");

        send(client_socket, buffer, BUFFER_SIZE, 0);
        recv(client_socket, buffer, BUFFER_SIZE, 0);
        choice = atoi(buffer);

        switch (choice) {
            case 1: // Change Password
                // Placeholder for implementation
                break;
            case 2: // View Account Balance
                // Placeholder for implementation
                break;
            case 3: // Deposit Money
                // Placeholder for implementation
                break;
            case 4: // Withdraw Money
                // Placeholder for implementation
                break;
            case 5: // Transfer Funds
                // Placeholder for implementation
                break;
            case 6: // Apply for a loan
                // Placeholder for implementation
                break;
            case 7: // View Transaction History
                // Placeholder for implementation
                break;
            case 8: // Provide Feedback
                // Placeholder for implementation
                break;
          
            default:
                send(client_socket, "Invalid choice!\n", BUFFER_SIZE, 0);
                break;
        }
    } while (choice != 9 && choice != 10);
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
                        " ____Enter logout to logout____\n"
                        " ____Enter exit to exit    ____\n"

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
            default:
                send(client_socket, "Invalid choice!\n", BUFFER_SIZE, 0);
                break;
        }
    } while (choice !=8 && choice != 9);
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
                        " ____Enter logout to logout____\n"
                        " ____Enter exit to exit    ____\n"

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
                        " ____Enter logout to logout____\n"
                        " ____Enter exit to exit    ____\n"

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
            default:
                send(client_socket, "Invalid choice!\n", BUFFER_SIZE, 0);
                break;
        }
    } while (choice != 6);  // Loop until Exit is selected
}

