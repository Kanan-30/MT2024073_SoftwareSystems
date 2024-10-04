#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <pthread.h>

#define PORT 5080
#define MAX_CLIENTS 10
#define BUFFER_SIZE 1024
#define DB_FILE "customer_db.txt"

typedef struct {
    char customer_id[20];
    char username[50];
    char password[50];
    double account_balance;
    int loan_numbers;
} Customer;


// Function prototypes
void *handle_client(void *arg);
void customer_menu(int client_socket);
void employee_menu(int client_socket);
void manager_menu(int client_socket);
void admin_menu(int client_socket);
//void save_customer_to_db(Customer *customer);

int customer_exists(const char *username);
int verify_customer(const char *username, const char *password);
void register_new_customer(int client_socket);
void customer_login(int client_socket) ;
// Structure to hold client details
typedef struct {
    int socket;
    struct sockaddr_in address;
} client_t;



int main() {
    int server_socket, client_socket;
    struct sockaddr_in server_address, client_address;
    socklen_t client_addr_len = sizeof(client_address);
    pthread_t thread_id;

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

        // Create a new thread for each client
        client_t *new_client = malloc(sizeof(client_t));
        new_client->socket = client_socket;
        new_client->address = client_address;
        
        if (pthread_create(&thread_id, NULL, handle_client, (void *)new_client) != 0) {
            perror("Thread creation failed");
            free(new_client);
        }
        pthread_detach(thread_id);
    }

    close(server_socket);
    return 0;
}

void *handle_client(void *arg) {
    client_t *client = (client_t *)arg;
    int client_socket = client->socket;
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

    close(client_socket);
    free(client);
    return NULL;
}
void save_customer_to_db(Customer *customer) {
    FILE *file = fopen(DB_FILE, "a");
    if (file == NULL) {
        perror("Unable to open customer database file");
        return;
    }

    fprintf(file, "%d %s %s %.2f %d\n",
            customer->id,
            customer->username,
            customer->password,
            customer->account_balance,
            customer->loan_numbers_applied);
    fclose(file);
}

int customer_exists(const char *username) {
    FILE *db_file = fopen(DB_FILE, "r");
    if (!db_file) {
        perror("Unable to open customer database");
        return 0; // Assume user doesn't exist if the database cannot be accessed
    }

    char line[BUFFER_SIZE];
    while (fgets(line, sizeof(line), db_file)) {
        // Assume each line in the db file contains "username,password,..."
        char *token = strtok(line, ",");
        if (token != NULL && strcmp(token, username) == 0) {
            fclose(db_file);
            return 1; // User exists
        }
    }

    fclose(db_file);
    return 0; // User does not exist
}


int verify_customer(const char *username, const char *password) {
    FILE *db_file = fopen(DB_FILE, "r");
    if (!db_file) {
        perror("Unable to open customer database");
        return 0; // Assume verification fails if the database cannot be accessed
    }

    char line[BUFFER_SIZE];
    while (fgets(line, sizeof(line), db_file)) {
        char *token = strtok(line, ",");
        if (token != NULL && strcmp(token, username) == 0) {
            // Assuming the second token is the password
            token = strtok(NULL, ",");
            if (token != NULL && strcmp(token, password) == 0) {
                fclose(db_file);
                return 1; // Credentials are valid
            }
        }
    }

    fclose(db_file);
    return 0; // Credentials are invalid
}


void register_new_customer(int client_socket) {
    char buffer[BUFFER_SIZE];
    Customer new_customer;

    // Collect customer details
    send(client_socket, "Enter Customer ID: ", BUFFER_SIZE, 0);
    recv(client_socket, buffer, BUFFER_SIZE, 0);
    strcpy(new_customer.customer_id, buffer);

    send(client_socket, "Enter Username: ", BUFFER_SIZE, 0);
    recv(client_socket, buffer, BUFFER_SIZE, 0);
    strcpy(new_customer.username, buffer);

    send(client_socket, "Enter Password: ", BUFFER_SIZE, 0);
    recv(client_socket, buffer, BUFFER_SIZE, 0);
    strcpy(new_customer.password, buffer);

    // Initial account balance and loan numbers
    new_customer.account_balance = 0.0;
    new_customer.loan_numbers = 0;

    // Store the new customer in the database
    FILE *file = fopen(DB_FILE, "a");
    fwrite(&new_customer, sizeof(Customer), 1, file);
    fclose(file);

    send(client_socket, "Registration successful!\n", BUFFER_SIZE, 0);
}

void customer_login(int client_socket) {
    char buffer[BUFFER_SIZE];
    char username[50], password[50];

    // Prompt the user to enter username
    send(client_socket, "Enter Username: ", BUFFER_SIZE, 0);
    recv(client_socket, username, sizeof(username), 0); // Receive username

    // Check if the customer is already registered
    if (customer_exists(username)) {
        // Existing customer, prompt for password
        send(client_socket, "Enter Password: ", BUFFER_SIZE, 0);
        recv(client_socket, password, sizeof(password), 0); // Receive password

        // Verify credentials
        if (verify_customer(username, password)) {
            send(client_socket, "Login successful!\n", BUFFER_SIZE, 0);
            send(client_socket, username, strlen(username) + 1, 0);  // Send username back to client
            // The customer menu will be shown after this function
        } else {
            send(client_socket, "Login failed! Invalid username or password.\n", BUFFER_SIZE, 0);
        }
    } else {
        // First-time user, prompt for registration
        send(client_socket, "You are a new customer. Registering you now...\n", BUFFER_SIZE, 0);
        register_new_customer(client_socket);
        send(client_socket, username, strlen(username) + 1, 0);  // Send username back to client
        // The customer menu will be shown after registration
    }
}

void customer_menu(int client_socket) {
     int choice;
    char buffer[BUFFER_SIZE];
    char username[50];  // To store the username after successful login
    int logged_in = 0;  // To track if the user has logged in


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
		customer_login(client_socket);
                recv(client_socket, username, 50, 0);  // Receive username after successful login
                logged_in = 1;  // Mark the user as logged in
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
		logged_in =0;
		return;
                break;
            case 11: // Exit
                send(client_socket, "Exiting...\n", BUFFER_SIZE, 0);
		close(client_socket); // Close the socket and exit
                pthread_exit(NULL); // Terminate the thread
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
                break;
            case 9: // Exit
                send(client_socket, "Exiting...\n", BUFFER_SIZE, 0);
		close(client_socket); // Close the socket and exit
                pthread_exit(NULL); // Terminate the thread
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
        
        send(client_socket, buffer, BUFFER_SIZE, 0);  // Send the manager menu to the client
        recv(client_socket, buffer, BUFFER_SIZE, 0);  // Receive the choice from the client
        choice = atoi(buffer);  // Convert the choice to integer

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
                pthread_exit(NULL);  // Terminate the thread
            
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
            case 1: // Implement Login System
                // Placeholder for implementation
                break;
            case 2: // Add New Bank Employee
                // Placeholder for implementation
                break;
            case 3: // Modify Customer Details
                // Placeholder for implementation
                break;
            case 4: // Modify Employee Details
                // Placeholder for implementation
                break;
            case 5: // Logout
                send(client_socket, "Logging out...\n", BUFFER_SIZE, 0);
                return;
		break;

            case 6: // Exit
                send(client_socket, "Exiting...\n", BUFFER_SIZE, 0);
		close(client_socket); // Close the socket and exit
                pthread_exit(NULL); // Terminate the thread
                break;
            default:
                send(client_socket, "Invalid choice!\n", BUFFER_SIZE, 0);
                break;
        }
    } while (choice != 5 && choice != 6);
}

