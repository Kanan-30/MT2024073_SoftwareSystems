#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <sys/file.h>  // For flock

#define PORT 5080
#define MAX_CLIENTS 10
#define BUFFER_SIZE 1024
#define CUSTOMER_DB "customer_db.txt"
#define EMPLOYEE_DB "employee_db.txt"
#define MANAGER_DB "manager_db.txt"
#define ADMIN_DB "admin_db.txt"
#define LOAN_DB "loans_db.txt"

typedef struct {
    int cust_id;
    char username[50];
    char password[50];
    double amount;
} Customer;

char logged_in_username[50];       // Global variable to track logged-in user
int logged_in_cust_id = -1;        // Global variable to track logged-in customer's ID
double logged_in_amount = 0.0;     // Global variable to track logged-in customer's amount

// Function prototypes
void handle_client(int client_socket);
void customer_menu(int client_socket);
void employee_menu(int client_socket);
void manager_menu(int client_socket);
void admin_menu(int client_socket);
int login_user(int client_socket, const char *role_file, const char *role);
int create_new_account(int client_socket, const char *role_file, const char *role);
void trim_newline(char *str);
int get_next_customer_id();
int update_customer_amount(int cust_id, double new_amount);
double get_customer_amount(int cust_id);
int customer_exists(int cust_id);
int transfer_funds(int client_socket);
int change_password(int client_socket);
int update_customer_password(int cust_id, const char* new_password);
int get_next_loan_id();
int apply_for_loan(int client_socket);

// Main function
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

// Function to handle client interactions
void handle_client(int client_socket) {
    char buffer[BUFFER_SIZE];
    int user_type;

    memset(buffer, 0, BUFFER_SIZE);  // Clear the buffer

    // Ask for user type
    send(client_socket, "Enter User Type (1: Customer, 2: Employee, 3: Manager, 4: Admin): ", strlen("Enter User Type (1: Customer, 2: Employee, 3: Manager, 4: Admin): "), 0);
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
            send(client_socket, "Invalid User Type!\n", strlen("Invalid User Type!\n"), 0);
            break;
    }
}

// Function to trim newline and carriage return from the buffer
void trim_newline(char *str) {
    int len = strlen(str);
    // Remove trailing newline
    if (len > 0 && str[len-1] == '\n') {
        str[len-1] = '\0';
        len--;
    }
    // Remove trailing carriage return (for Windows clients)
    if (len > 0 && str[len-1] == '\r') {
        str[len-1] = '\0';
    }
}

// Function to get the next customer ID
int get_next_customer_id() {
    FILE *file = fopen(CUSTOMER_DB, "r");
    if (!file) {
        // If file doesn't exist, start with ID 1
        return 1;
    }

    char line[BUFFER_SIZE];
    int last_id = 0;
    int cust_id;
    while (fgets(line, sizeof(line), file)) {
        if (sscanf(line, "%d", &cust_id) == 1) {
            if (cust_id > last_id) {
                last_id = cust_id;
            }
        }
    }
    fclose(file);
    return last_id + 1;
}

// Function to check if a customer exists by cust_id
int customer_exists(int cust_id) {
    FILE *file = fopen(CUSTOMER_DB, "r");
    if (!file) {
        return 0;
    }

    char line[BUFFER_SIZE];
    int current_id;
    while (fgets(line, sizeof(line), file)) {
        if (sscanf(line, "%d", &current_id) == 1) {
            if (current_id == cust_id) {
                fclose(file);
                return 1;
            }
        }
    }
    fclose(file);
    return 0;
}

// Function to get customer's current amount
double get_customer_amount(int cust_id) {
    FILE *file = fopen(CUSTOMER_DB, "r");
    if (!file) {
        perror("Error opening customer database for reading amount");
        return -1.0;
    }

    char line[BUFFER_SIZE];
    double amount = -1.0;
    while (fgets(line, sizeof(line), file)) {
        int current_id;
        double current_amount;
        char username[50], password[50];

        int parsed = sscanf(line, "%d %s %s %lf", &current_id, username, password, &current_amount);
        if (parsed < 4) continue;  // Invalid line format

        if (current_id == cust_id) {
            amount = current_amount;
            break;
        }
    }
    fclose(file);
    return amount;
}

// Function to update customer's amount in the database
int update_customer_amount(int cust_id, double new_amount) {
    FILE *file = fopen(CUSTOMER_DB, "r+");
    if (!file) {
        perror("Error opening customer database for updating");
        return 0;
    }

    // Lock the file for writing
    if (flock(fileno(file), LOCK_EX) < 0) {
        perror("Error locking the file");
        fclose(file);
        return 0;
    }

    char line[BUFFER_SIZE];
    long pos;
    int updated = 0;
    while ((pos = ftell(file)) != -1 && fgets(line, sizeof(line), file)) {
        int current_id;
        double current_amount;
        char username[50], password[50];

        int parsed = sscanf(line, "%d %s %s %lf", &current_id, username, password, &current_amount);
        if (parsed < 4) continue;  // Invalid line format

        if (current_id == cust_id) {
            // Move the file pointer back to the start of the amount field
            // Assuming the format is: cust_id username password amount\n
            // We need to find the position after the third space
            int space_count = 0;
            int i;
            for (i = 0; line[i] != '\0'; i++) {
                if (line[i] == ' ') {
                    space_count++;
                    if (space_count == 3) {
                        break;
                    }
                }
            }

            if (space_count < 3) {
                fclose(file);
                return 0;  // Invalid format
            }

            // Prepare the new amount string
            char new_amount_str[20];
            snprintf(new_amount_str, sizeof(new_amount_str), "%.2lf\n", new_amount);

            // Overwrite the existing amount
            fseek(file, pos + i + 1, SEEK_SET);  // +1 to skip the space
            fputs(new_amount_str, file);
            fflush(file);  // Ensure data is written to disk
            updated = 1;
            break;
        }
    }

    // Unlock and close the file
    flock(fileno(file), LOCK_UN);
    fclose(file);

    return updated;
}

// Function to update customer's password in the database
int update_customer_password(int cust_id, const char* new_password) {
    FILE *file = fopen(CUSTOMER_DB, "r");
    if (!file) {
        perror("Error opening customer database for reading");
        return 0;
    }

    // Open a temporary file for writing
    FILE *temp = fopen("customer_db.tmp", "w");
    if (!temp) {
        perror("Error opening temporary file for writing");
        fclose(file);
        return 0;
    }

    // Lock the original file for reading
    if (flock(fileno(file), LOCK_SH) < 0) {
        perror("Error locking the original file");
        fclose(file);
        fclose(temp);
        return 0;
    }

    char line[BUFFER_SIZE];
    int updated = 0;

    while (fgets(line, sizeof(line), file)) {
        int current_id;
        char username[50], password[50];
        double amount;

        int parsed = sscanf(line, "%d %s %s %lf", &current_id, username, password, &amount);
        if (parsed < 4) {
            // Write the line as is if format is invalid
            fputs(line, temp);
            continue;
        }

        if (current_id == cust_id) {
            // Replace the password
            fprintf(temp, "%d %s %s %.2lf\n", current_id, username, new_password, amount);
            updated = 1;
        } else {
            // Write the line as is
            fputs(line, temp);
        }
    }

    // Unlock and close the original file
    flock(fileno(file), LOCK_UN);
    fclose(file);

    // Close the temporary file
    fclose(temp);

    if (!updated) {
        // Remove the temporary file if no update was done
        remove("customer_db.tmp");
        return 0;
    }

    // Replace the original file with the temporary file
    if (rename("customer_db.tmp", CUSTOMER_DB) != 0) {
        perror("Error renaming temporary file");
        return 0;
    }

    return 1;
}

// Function to get the next loan ID in the format loan_1, loan_2, etc.
int get_next_loan_id() {
    FILE *file = fopen(LOAN_DB, "r");
    if (!file) {
        // If file doesn't exist, start with loan_1
        return 1;
    }

    char line[BUFFER_SIZE];
    int last_id = 0;
    int loan_id_num;
    char loan_id_str[20];
    while (fgets(line, sizeof(line), file)) {
        if (sscanf(line, "loan_%d", &loan_id_num) == 1) {
            if (loan_id_num > last_id) {
                last_id = loan_id_num;
            }
        }
    }
    fclose(file);
    return last_id + 1;
}

// Function to login user from a file
int login_user(int client_socket, const char *role_file, const char *role) {
    char buffer[BUFFER_SIZE], username[50], password[50], file_username[50], file_password[50];
    char line[BUFFER_SIZE];
    int file_fd;

    memset(buffer, 0, BUFFER_SIZE);        // Clear the buffer
    memset(username, 0, sizeof(username)); // Clear username
    memset(password, 0, sizeof(password)); // Clear password

    // Ask for username
    send(client_socket, "Enter Username: ", strlen("Enter Username: "), 0);
    recv(client_socket, username, sizeof(username), 0);
    trim_newline(username); // Clean input

    // Open the role file (e.g., customer, employee file)
    file_fd = open(role_file, O_RDONLY);
    if (file_fd < 0) {
        perror("Error opening file");
        send(client_socket, "Server error. Please try again later.\n", strlen("Server error. Please try again later.\n"), 0);
        return 0;
    }

    FILE *file = fdopen(file_fd, "r");
    if (!file) {
        perror("fdopen failed");
        close(file_fd);
        send(client_socket, "Server error. Please try again later.\n", strlen("Server error. Please try again later.\n"), 0);
        return 0;
    }

    int found = 0;
    char current_cust_id_str[20] = "";
    int current_cust_id = -1;
    double current_amount = 0.0;

    // Read each line and check if the username exists
    while (fgets(line, sizeof(line), file)) {
        if (strcmp(role, "Customer") == 0) {
            // For customers, lines are: cust_id username password amount
            int parsed = sscanf(line, "%s %s %s %lf", current_cust_id_str, file_username, file_password, &current_amount);
            if (parsed < 4) continue; // Invalid line format
            current_cust_id = atoi(current_cust_id_str);
        } else {
            // For other roles, lines are: username password
            int parsed = sscanf(line, "%s %s", file_username, file_password);
            if (parsed < 2) continue; // Invalid line format
        }

        if (strcmp(file_username, username) == 0) {
            found = 1; // Username found
            break;
        }
    }
    fclose(file); // This also closes file_fd

    if (found) {
        // Ask for password if username exists
        memset(password, 0, sizeof(password));
        send(client_socket, "Enter Password: ", strlen("Enter Password: "), 0);
        recv(client_socket, password, sizeof(password), 0);
        trim_newline(password); // Clean input

        if (strcmp(file_password, password) == 0) {
            send(client_socket, "Login successful!\n", strlen("Login successful!\n"), 0);
            if (strcmp(role, "Customer") == 0) {
                // For customers, store cust_id and amount
                logged_in_cust_id = current_cust_id;
                logged_in_amount = current_amount;
            }
            strcpy(logged_in_username, username);
            return 1; // Successful login
        } else {
            send(client_socket, "Login failed! Incorrect password.\n", strlen("Login failed! Incorrect password.\n"), 0);
            return 0; // Password mismatch
        }
    } else {
        // If username not found, prompt to create a new account
        send(client_socket, "Username not found! Create a new account? (yes/no): ", strlen("Username not found! Create a new account? (yes/no): "), 0);
        recv(client_socket, buffer, sizeof(buffer), 0);
        trim_newline(buffer); // Clean input

        if (strcasecmp(buffer, "yes") == 0) {
            return create_new_account(client_socket, role_file, role); // Create account if yes
        } else {
            send(client_socket, "Login cancelled.\n", strlen("Login cancelled.\n"), 0);
            return 0; // Cancel if no
        }
    }
}

// Function to create a new account
int create_new_account(int client_socket, const char *role_file, const char *role) {
    char username[50], password[50];
    int file_fd;

    if (strcmp(role, "Customer") == 0) {
        // For customers, open file in append mode
        file_fd = open(role_file, O_WRONLY | O_APPEND | O_CREAT, 0644);
        if (file_fd < 0) {
            perror("Error opening customer file");
            send(client_socket, "Server error. Please try again later.\n", strlen("Server error. Please try again later.\n"), 0);
            return 0;
        }

        FILE *file = fdopen(file_fd, "a");
        if (!file) {
            perror("fdopen failed");
            close(file_fd);
            send(client_socket, "Server error. Please try again later.\n", strlen("Server error. Please try again later.\n"), 0);
            return 0;
        }

        memset(username, 0, sizeof(username)); // Clear username
        memset(password, 0, sizeof(password)); // Clear password

        // Get the new username and password
        send(client_socket, "Enter new Username: ", strlen("Enter new Username: "), 0);
        recv(client_socket, username, sizeof(username), 0);
        trim_newline(username); // Clean input

        send(client_socket, "Enter new Password: ", strlen("Enter new Password: "), 0);
        recv(client_socket, password, sizeof(password), 0);
        trim_newline(password); // Clean input

        // Generate a new customer ID
        int cust_id = get_next_customer_id();

        // Initial amount is 0.00
        double initial_amount = 0.00;

        // Save the new customer to the file (cust_id username password amount)
        fprintf(file, "%d %s %s %.2lf\n", cust_id, username, password, initial_amount);
        fclose(file); // This also closes file_fd

        char success_msg[BUFFER_SIZE];
        snprintf(success_msg, sizeof(success_msg), "Account created successfully! Your Customer ID is %d.\n", cust_id);
        send(client_socket, success_msg, strlen(success_msg), 0);
        return 1;
    } else {
        // For other roles, similar to existing behavior
        file_fd = open(role_file, O_WRONLY | O_APPEND | O_CREAT, 0644);
        if (file_fd < 0) {
            perror("Error opening file");
            send(client_socket, "Server error. Please try again later.\n", strlen("Server error. Please try again later.\n"), 0);
            return 0;
        }

        FILE *file = fdopen(file_fd, "a");
        if (!file) {
            perror("fdopen failed");
            close(file_fd);
            send(client_socket, "Server error. Please try again later.\n", strlen("Server error. Please try again later.\n"), 0);
            return 0;
        }

        memset(username, 0, sizeof(username)); // Clear username
        memset(password, 0, sizeof(password)); // Clear password

        // Get the new username and password
        send(client_socket, "Enter new Username: ", strlen("Enter new Username: "), 0);
        recv(client_socket, username, sizeof(username), 0);
        trim_newline(username); // Clean input

        send(client_socket, "Enter new Password: ", strlen("Enter new Password: "), 0);
        recv(client_socket, password, sizeof(password), 0);
        trim_newline(password); // Clean input

        // Save the new user to the file (username password)
        fprintf(file, "%s %s\n", username, password);
        fclose(file); // This also closes file_fd

        send(client_socket, "Account created successfully!\n", strlen("Account created successfully!\n"), 0);
        return 1;
    }
}

// Function to transfer funds from one customer to another
int transfer_funds(int client_socket) {
    char buffer[BUFFER_SIZE];
    int target_cust_id;
    double transfer_amount;

    // Prompt for target customer ID
    send(client_socket, "Enter the Customer ID to transfer to: ", strlen("Enter the Customer ID to transfer to: "), 0);
    recv(client_socket, buffer, sizeof(buffer), 0);
    trim_newline(buffer);
    target_cust_id = atoi(buffer);

    if (target_cust_id <= 0) {
        send(client_socket, "Invalid Customer ID.\n", strlen("Invalid Customer ID.\n"), 0);
        return 0;
    }

    if (target_cust_id == logged_in_cust_id) {
        send(client_socket, "Cannot transfer funds to the same account.\n", strlen("Cannot transfer funds to the same account.\n"), 0);
        return 0;
    }

    // Check if target customer exists
    if (!customer_exists(target_cust_id)) {
        send(client_socket, "Target Customer ID does not exist.\n", strlen("Target Customer ID does not exist.\n"), 0);
        return 0;
    }

    // Prompt for transfer amount
    send(client_socket, "Enter amount to transfer: ", strlen("Enter amount to transfer: "), 0);
    recv(client_socket, buffer, sizeof(buffer), 0);
    trim_newline(buffer);
    transfer_amount = atof(buffer);

    if (transfer_amount <= 0) {
        send(client_socket, "Invalid transfer amount.\n", strlen("Invalid transfer amount.\n"), 0);
        return 0;
    }

    // Get current balance of sender
    double sender_balance = get_customer_amount(logged_in_cust_id);
    if (sender_balance < 0) {
        send(client_socket, "Error retrieving your account balance.\n", strlen("Error retrieving your account balance.\n"), 0);
        return 0;
    }

    if (sender_balance < transfer_amount) {
        send(client_socket, "Insufficient funds for the transfer.\n", strlen("Insufficient funds for the transfer.\n"), 0);
        return 0;
    }

    // Get current balance of receiver
    double receiver_balance = get_customer_amount(target_cust_id);
    if (receiver_balance < 0) {
        send(client_socket, "Error retrieving receiver's account balance.\n", strlen("Error retrieving receiver's account balance.\n"), 0);
        return 0;
    }

    // Update balances
    double new_sender_balance = sender_balance - transfer_amount;
    double new_receiver_balance = receiver_balance + transfer_amount;

    // Perform updates with file locking to ensure atomicity
    if (!update_customer_amount(logged_in_cust_id, new_sender_balance)) {
        send(client_socket, "Error updating your account balance.\n", strlen("Error updating your account balance.\n"), 0);
        return 0;
    }

    if (!update_customer_amount(target_cust_id, new_receiver_balance)) {
        // Attempt to rollback sender's balance if receiver's update fails
        update_customer_amount(logged_in_cust_id, sender_balance);
        send(client_socket, "Error updating receiver's account balance. Transfer rolled back.\n", strlen("Error updating receiver's account balance. Transfer rolled back.\n"), 0);
        return 0;
    }

    send(client_socket, "Transfer successful!\n", strlen("Transfer successful!\n"), 0);
    return 1;
}

// Function to change the password of the logged-in customer
int change_password(int client_socket) {
    char buffer[BUFFER_SIZE];
    char new_password[50];

    // Prompt for new password
    send(client_socket, "Enter your new password: ", strlen("Enter your new password: "), 0);
    recv(client_socket, new_password, sizeof(new_password), 0);
    trim_newline(new_password); // Clean input

    // Validate new password
    if (strlen(new_password) == 0) {
        send(client_socket, "Password cannot be empty.\n", strlen("Password cannot be empty.\n"), 0);
        return 0;
    }

    // Update the password in the database
    if (update_customer_password(logged_in_cust_id, new_password)) {
        send(client_socket, "Password changed successfully!\n", strlen("Password changed successfully!\n"), 0);
        return 1;
    } else {
        send(client_socket, "Error changing password. Please try again.\n", strlen("Error changing password. Please try again.\n"), 0);
        return 0;
    }
}
/*void trim_newline(char *str) {
    int len = strlen(str);
    if (len > 0 && str[len - 1] == '\n') {
        str[len - 1] = '\0';
    }
}*/

// Function to apply for a loan
int apply_for_loan(int client_socket) {
    char buffer[BUFFER_SIZE], loan_amount[20];
    int cust_id, file, loan_id = 1;
    char line[BUFFER_SIZE];
    char file_username[50], file_password[50];
    float balance;
    char loans[BUFFER_SIZE] = "";
    char updated_loans[BUFFER_SIZE] = "";

    // Prompt the customer for their ID
    send(client_socket, "Enter your customer ID: ", strlen("Enter your customer ID: "), 0);
    recv(client_socket, buffer, BUFFER_SIZE, 0);
    cust_id = atoi(buffer);

    // Open customer_db file
    file = open(CUSTOMER_DB, O_RDWR);
    if (file < 0) {
        perror("Error opening customer database");
        return 0;
    }

    int found = 0;
    off_t pos;
    FILE *file_stream = fdopen(file, "r+");  // Open file as stream for easier line-by-line reading
    if (file_stream == NULL) {
        perror("Error reading file stream");
        close(file);
        return 0;
    }

    // Search for the customer in the file
    while (fgets(line, sizeof(line), file_stream) != NULL) {
        pos = ftell(file_stream);  // Save the current position for potential update
        int temp_cust_id;
        sscanf(line, "%d %s %s %f %[^\n]", &temp_cust_id, file_username, file_password, &balance, loans);
        if (temp_cust_id == cust_id) {
            found = 1;
            break;
        }
    }

    if (!found) {
        send(client_socket, "Customer ID not found!\n", strlen("Customer ID not found!\n"), 0);
        fclose(file_stream);
        return 0;
    }

    // Prompt the customer for the loan amount
    send(client_socket, "Enter loan amount: ", strlen("Enter loan amount: "), 0);
    recv(client_socket, loan_amount, sizeof(loan_amount), 0);
    trim_newline(loan_amount);  // Clean input
    float loan_amt = atof(loan_amount);

    // Determine the next loan_id by counting existing loans
    if (strlen(loans) > 0) {
        char *loan_ptr = strtok(loans, ",");
        while (loan_ptr != NULL) {
            loan_ptr = strtok(NULL, ",");
            loan_id++;
        }
    }

    // Construct the new loan entry (loan_id:amount)
    char new_loan[BUFFER_SIZE];
    snprintf(new_loan, sizeof(new_loan), "loan_%d:%.2f", loan_id, loan_amt);

    // Append new loan to the existing loans or create a new loans entry
    if (strlen(loans) > 0) {
        snprintf(updated_loans, sizeof(updated_loans), "%s,%s", loans, new_loan);
    } else {
        snprintf(updated_loans, sizeof(updated_loans), "%s", new_loan);
    }

    // Move the file pointer back to the beginning of the customer record
    fseek(file_stream, pos - strlen(line), SEEK_SET);

    // Update the customer record in the file
    snprintf(buffer, sizeof(buffer), "%d %s %s %.2f %s\n", cust_id, file_username, file_password, balance, updated_loans);
    fputs(buffer, file_stream);

    send(client_socket, "Loan applied successfully!\n", strlen("Loan applied successfully!\n"), 0);

    fclose(file_stream);
    return 1;
}

// Function to implement the customer menu
void customer_menu(int client_socket) {
    int choice;
    char buffer[BUFFER_SIZE];

    do {
        snprintf(buffer, sizeof(buffer),
                "\nCustomer Menu:\n"
                "1. Change Password\n"
                "2. View Account Balance\n"
                "3. Deposit Money\n"
                "4. Withdraw Money\n"
                "5. Transfer Funds\n"
                "6. Apply for a Loan\n"
                "7. Logout\n"
                "Enter your choice: ");
        send(client_socket, buffer, strlen(buffer), 0);
        memset(buffer, 0, BUFFER_SIZE);
        recv(client_socket, buffer, sizeof(buffer), 0);
        choice = atoi(buffer);

        switch (choice) {
            case 1:
                // Change Password
                if (!change_password(client_socket)) {
                    // Message already sent within change_password()
                }
                break;
            case 2:
                // View Account Balance
                {
                    double balance = get_customer_amount(logged_in_cust_id);
                    if (balance >= 0.0) {
                        char balance_msg[BUFFER_SIZE];
                        snprintf(balance_msg, sizeof(balance_msg), "Your current account balance is: $%.2lf\n", balance);
                        send(client_socket, balance_msg, strlen(balance_msg), 0);
                    } else {
                        send(client_socket, "Error retrieving account balance.\n", strlen("Error retrieving account balance.\n"), 0);
                    }
                }
                break;
            case 3:
                // Deposit Money
                {
                    double deposit_amount;
                    send(client_socket, "Enter amount to deposit: ", strlen("Enter amount to deposit: "), 0);
                    recv(client_socket, buffer, sizeof(buffer), 0);
                    deposit_amount = atof(buffer);
                    if (deposit_amount <= 0) {
                        send(client_socket, "Invalid deposit amount.\n", strlen("Invalid deposit amount.\n"), 0);
                    } else {
                        double current_balance = get_customer_amount(logged_in_cust_id);
                        if (current_balance < 0) {
                            send(client_socket, "Error retrieving current balance.\n", strlen("Error retrieving current balance.\n"), 0);
                        } else {
                            double new_balance = current_balance + deposit_amount;
                            if (update_customer_amount(logged_in_cust_id, new_balance)) {
                                send(client_socket, "Deposit successful!\n", strlen("Deposit successful!\n"), 0);
                            } else {
                                send(client_socket, "Error updating account balance.\n", strlen("Error updating account balance.\n"), 0);
                            }
                        }
                    }
                }
                break;
            case 4:
                // Withdraw Money
                {
                    double withdraw_amount;
                    send(client_socket, "Enter amount to withdraw: ", strlen("Enter amount to withdraw: "), 0);
                    recv(client_socket, buffer, sizeof(buffer), 0);
                    withdraw_amount = atof(buffer);
                    if (withdraw_amount <= 0) {
                        send(client_socket, "Invalid withdraw amount.\n", strlen("Invalid withdraw amount.\n"), 0);
                    } else {
                        double current_balance = get_customer_amount(logged_in_cust_id);
                        if (current_balance < 0) {
                            send(client_socket, "Error retrieving current balance.\n", strlen("Error retrieving current balance.\n"), 0);
                        } else if (withdraw_amount > current_balance) {
                            send(client_socket, "Insufficient funds.\n", strlen("Insufficient funds.\n"), 0);
                        } else {
                            double new_balance = current_balance - withdraw_amount;
                            if (update_customer_amount(logged_in_cust_id, new_balance)) {
                                send(client_socket, "Withdrawal successful!\n", strlen("Withdrawal successful!\n"), 0);
                            } else {
                                send(client_socket, "Error updating account balance.\n", strlen("Error updating account balance.\n"), 0);
                            }
                        }
                    }
                }
                break;
            case 5:
                // Transfer Funds
                if (!transfer_funds(client_socket)) {
                    // Transfer failed; message already sent within transfer_funds()
                }
                break;
            case 6:
                // Apply for a Loan
                if (!apply_for_loan(client_socket)) {
                    // Message already sent within apply_for_loan()
                }
                break;
            case 7:
                send(client_socket, "Logged out.\n", strlen("Logged out.\n"), 0);
                break;
            default:
                send(client_socket, "Invalid choice! Please try again.\n", strlen("Invalid choice! Please try again.\n"), 0);
                break;
        }
    } while (choice != 7);
}

// Function to implement employee menu
void employee_menu(int client_socket) {
    // Implement employee menu
    send(client_socket, "Employee Menu is not implemented yet.\n", strlen("Employee Menu is not implemented yet.\n"), 0);
}

// Function to implement manager menu
void manager_menu(int client_socket) {
    // Implement manager menu
    send(client_socket, "Manager Menu is not implemented yet.\n", strlen("Manager Menu is not implemented yet.\n"), 0);
}

// Function to implement admin menu
void admin_menu(int client_socket) {
    // Implement admin menu
    send(client_socket, "Admin Menu is not implemented yet.\n", strlen("Admin Menu is not implemented yet.\n"), 0);
}

