#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <sys/file.h> 
#include <time.h>

#define PORT 5080
#define MAX_CLIENTS 10
#define BUFFER_SIZE 1048
#define CUSTOMER_DB "customer_db.txt"
#define EMPLOYEE_DB "employee_db.txt"
#define MANAGER_DB "manager_db.txt"
#define ADMIN_DB "admin_db.txt"
#define LOAN_DB "loans_db.txt"
#define CUSTOMER_STATUS_FILE "customer_status.txt"
#define LOAN_ASSIGNMENTS_FILE "loan_assignments.txt"
#define FEEDBACK_FILE "employee_feedback.txt" 

typedef struct {
    int cust_id;
    char username[50];
    char password[50];
    double amount;
} Customer;

typedef struct {
    char emp_id[10];     
    char username[50];
    char password[50];
} Employee;


char logged_in_username[50];       
int logged_in_cust_id = -1;        
double logged_in_amount = 000.00;   
char logged_in_emp_id[10] = ""; 

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
int get_next_loan_id();
int apply_for_loan(int client_socket);
int update_customer_status_func(int client_socket, int cust_id, int new_status);
int assign_loan_to_employee_func(int client_socket, int loan_id, const char *employee_id);
int get_next_employee_id_num();
int add_new_employee(int client_socket);
int view_assigned_loans(int client_socket);
void approve_or_reject_loan(int client_socket) ;
int check_loan_status(int client_socket, int customer_id);
int change_customer_password(int client_socket);
int change_employee_password(int client_socket);
int update_employee_password(const char* emp_id, const char* new_password);
void give_feedback_to_employee(int client_socket, const char *employee_db, const char *feedback_file);
void view_employee_feedback(int client_socket, const char *feedback_file);
void log_transaction(int cust_id, const char *transaction_type, double amount);
void view_transaction_history(int client_socket, int cust_id) ;
void view_customer_transaction_history(int client_socket, int cust_id) ;

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


int login_user(int client_socket, const char *role_file, const char *role) {
    char buffer[BUFFER_SIZE], username[50], password[50], file_emp_id[10], file_username[50], file_password[50];
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
        send(client_socket, "Server error. Please try again later.\n",
             strlen("Server error. Please try again later.\n"), 0);
        return 0;
    }

    FILE *file = fdopen(file_fd, "r");
    if (!file) {
        perror("fdopen failed");
        close(file_fd);
        send(client_socket, "Server error. Please try again later.\n",
             strlen("Server error. Please try again later.\n"), 0);
        return 0;
    }

    int found = 0;
    int current_cust_id = -1;
    double current_amount = 000.00;
    char current_emp_id[10] = "";

    // Read each line and check if the username exists
    while (fgets(line, sizeof(line), file)) {
        if (strcmp(role, "Customer") == 0) {
            // For customers, lines are: cust_id username password amount
            int parsed = sscanf(line, "%d %s %s %lf", &current_cust_id, file_username, file_password, &current_amount);
            if (parsed < 4) continue; // Invalid line format
        } else if (strcmp(role, "Employee") == 0) {
            // For employees, lines are: emp_id username password
            int parsed = sscanf(line, "%s %s %s", file_emp_id, file_username, file_password);
            if (parsed < 3) continue; // Invalid line format
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
            if (strcmp(role, "Employee") == 0) {
                // Store emp_id in global variable
                strcpy(logged_in_emp_id, file_emp_id);
            }
            return 1; // Successful login
        } else {
            send(client_socket, "Login failed! Incorrect password.\n",
                 strlen("Login failed! Incorrect password.\n"), 0);
            return 0; // Password mismatch
        }
    } else {
        // If username not found, prompt to create a new account
        send(client_socket, "Username not found! Create a new account? (yes/no): ",
             strlen("Username not found! Create a new account? (yes/no): "), 0);
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
        double initial_amount = 000.00;

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

    // Log the transfer transaction for both sender and receiver
    log_transaction(logged_in_cust_id, "transfer_sent", transfer_amount);
    log_transaction(target_cust_id, "transfer_received", transfer_amount);

    return 1;
}
void view_transaction_history(int client_socket, int cust_id) {
    FILE *file = fopen("transactions_db.txt", "r");
    if (!file) {
        perror("Error opening transaction log file");
        send(client_socket, "Error retrieving transaction history.\n", strlen("Error retrieving transaction history.\n"), 0);
        return;
    }

    char line[BUFFER_SIZE];
    char response[BUFFER_SIZE * 10] = "Transaction History:\n";
    int has_transactions = 0;

    while (fgets(line, sizeof(line), file)) {
        int current_id;
        char transaction_type[50];  // Increased size to handle longer transaction types
        double amount;
        char timestamp[50];

        // Updated sscanf format string to handle spaces and commas properly
        sscanf(line, "%d, %49[^,], %lf, %49[^\n]", &current_id, transaction_type, &amount, timestamp);

        if (current_id == cust_id) {
            has_transactions = 1;
            char transaction_details[BUFFER_SIZE];
            snprintf(transaction_details, sizeof(transaction_details), "%s: %.2lf on %s\n", transaction_type, amount, timestamp);
            strcat(response, transaction_details);
        }
    }

    fclose(file);

    if (!has_transactions) {
        strcat(response, "No transactions found.\n");
    }

    send(client_socket, response, strlen(response), 0);
}

int apply_for_loan(int client_socket) {
    char buffer[BUFFER_SIZE], loan_amount[20];
    int cust_id, loan_id;
    char line[BUFFER_SIZE];
    char file_username[50], file_password[50];
    float balance;

    // Prompt the customer for their ID
    send(client_socket, "Enter your customer ID: ", strlen("Enter your customer ID: "), 0);
    recv(client_socket, buffer, BUFFER_SIZE, 0);
    cust_id = atoi(buffer);

    // Open customer_db file to verify customer
    int file = open(CUSTOMER_DB, O_RDWR);
    if (file < 0) {
        perror("Error opening customer database");
        return 0;
    }

    int found = 0;
    FILE *file_stream = fdopen(file, "r+");  // Open file as stream for easier line-by-line reading
    if (file_stream == NULL) {
        perror("Error reading file stream");
        close(file);
        return 0;
    }

    // Search for the customer in the file
    while (fgets(line, sizeof(line), file_stream) != NULL) {
        int temp_cust_id;
        sscanf(line, "%d %s %s %f", &temp_cust_id, file_username, file_password, &balance);
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

    // Get the next loan ID from loan_db
    loan_id = get_next_loan_id();

    // Open loan_db to append the new loan
    FILE *loan_db_file = fopen(LOAN_DB, "a");
    if (!loan_db_file) {
        perror("Error opening loan database");
        fclose(file_stream);
        return 0;
    }

    // Construct the loan record (loan_id, cust_id, loan_amount)
    snprintf(buffer, sizeof(buffer), "loan_%d %d %.2f\n", loan_id, cust_id, loan_amt);

    // Write the loan record to the loan_db file
    fputs(buffer, loan_db_file);
    fclose(loan_db_file);

    // Notify the customer that the loan was applied successfully
    send(client_socket, "Loan applied successfully!\n", strlen("Loan applied successfully!\n"), 0);

    fclose(file_stream);
    return 1;
}
int check_loan_status(int client_socket, int customer_id) {
    FILE *loan_db_file = fopen(LOAN_DB, "r");
    if (!loan_db_file) {
        perror("Error opening loan database file");
        send(client_socket, "Error opening loan database file.\n", 
             strlen("Error opening loan database file.\n"), 0);
        return 0;
    }

    // Prepare response message for customer
    char response[BUFFER_SIZE * 10] = "Your Loan Status:\nLoan ID\tAmount\tStatus\n";

    // Buffer to read the file line by line
    char line[BUFFER_SIZE];
    int loan_found = 0;

    // Scan through the loans database file
    while (fgets(line, sizeof(line), loan_db_file)) {
        int loan_id, db_cust_id;
        double amount;

        // Parse loan_id, customer_id, and amount
        if (sscanf(line, "loan_%d %d %lf", &loan_id, &db_cust_id, &amount) == 3) {
            if (db_cust_id == customer_id) {
                loan_found = 1;

                // Check loan status in loan_assignments.txt
                FILE *assignment_file = fopen(LOAN_ASSIGNMENTS_FILE, "r");
                if (!assignment_file) {
                    perror("Error opening loan assignment file");
                    send(client_socket, "Error opening loan assignment file.\n", 
                         strlen("Error opening loan assignment file.\n"), 0);
                    fclose(loan_db_file);
                    return 0;
                }

                char assignment_line[BUFFER_SIZE];
                char status[20] = "Not Assigned";  // Default status if not found
                while (fgets(assignment_line, sizeof(assignment_line), assignment_file)) {
                    int assigned_loan_id;
                    char emp_id[50], temp_status[20];

                    // Parse assignment file for loan and status
                    if (sscanf(assignment_line, "loan_%d %s %s", &assigned_loan_id, emp_id, temp_status) == 3) {
                        if (assigned_loan_id == loan_id) {
                            strncpy(status, temp_status, sizeof(status) - 1);
                            status[sizeof(status) - 1] = '\0';  // Ensure null-termination
                            break;
                        }
                    }
                }
                fclose(assignment_file);

                // Append loan information and status to the response
                char loan_info[BUFFER_SIZE];
                snprintf(loan_info, sizeof(loan_info), "loan_%d\t%.2lf\t%s\n", loan_id, amount, status);
                strncat(response, loan_info, sizeof(response) - strlen(response) - 1);
            }
        }
    }
    fclose(loan_db_file);

    if (!loan_found) {
        send(client_socket, "No loans found for your account.\n", 
             strlen("No loans found for your account.\n"), 0);
    } else {
        // Send the response to the customer
        send(client_socket, response, strlen(response), 0);
    }

    return 1;
}
int change_customer_password(int client_socket) {
    char buffer[BUFFER_SIZE];
    char new_password[50];

    // Prompt for the new password
    send(client_socket, "Enter your new password: ", strlen("Enter your new password: "), 0);
    recv(client_socket, new_password, sizeof(new_password), 0);
    trim_newline(new_password); // Clean input

    // Validate new password
    if (strlen(new_password) == 0) {
        send(client_socket, "Password cannot be empty.\n", strlen("Password cannot be empty.\n"), 0);
        return 0;
    }

    // Open the customer database file
    FILE *file = fopen(CUSTOMER_DB, "r");
    if (!file) {
        perror("Error opening customer database");
        send(client_socket, "Error opening customer database.\n", strlen("Error opening customer database.\n"), 0);
        return 0;
    }

    // Open a temporary file to write the updated content
    FILE *temp = fopen("customer_db.tmp", "w");
    if (!temp) {
        perror("Error opening temporary file");
        fclose(file);
        send(client_socket, "Error opening temporary file.\n", strlen("Error opening temporary file.\n"), 0);
        return 0;
    }

    char line[BUFFER_SIZE];
    int updated = 0;

    // Loop through the file to find the matching username
    while (fgets(line, sizeof(line), file)) {
        char username[50], password[50];
        int cust_id;
        double amount;

        sscanf(line, "%d %s %s %lf", &cust_id, username, password, &amount);

        if (strcmp(username, logged_in_username) == 0) {
            // Update the password for the logged-in user
            fprintf(temp, "%d %s %s %.2lf\n", cust_id, username, new_password, amount);
            updated = 1;
        } else {
            // Write the line as-is
            fputs(line, temp);
        }
    }

    fclose(file);
    fclose(temp);

    // If the password was updated, rename the temp file to replace the original file
    if (updated) {
        rename("customer_db.tmp", CUSTOMER_DB);
        send(client_socket, "Password changed successfully!\n", strlen("Password changed successfully!\n"), 0);
        return 1;
    } else {
        // Clean up the temporary file if no changes were made
        remove("customer_db.tmp");
        send(client_socket, "Error: Username not found.\n", strlen("Error: Username not found.\n"), 0);
        return 0;
    }
}
void trim_newline_and_control_chars(char *str) {
    int len = strlen(str);
    // Remove trailing newline, carriage return, and other control characters
    while (len > 0 && (str[len-1] == '\n' || str[len-1] == '\r' || str[len-1] == '\t' || str[len-1] == '\b')) {
        str[len-1] = '\0';
        len--;
    }
}

// Function for customers to give feedback to employees
void give_feedback_to_employee(int client_socket, const char *employee_db, const char *feedback_file) {
    char line[BUFFER_SIZE], emp_id[20], emp_username[50];
    char selected_emp_id[20], feedback[BUFFER_SIZE];
    char buffer[BUFFER_SIZE];
    int file_fd;

    // Open employee_db to display list of employees
    file_fd = open(employee_db, O_RDONLY);
    if (file_fd < 0) {
        perror("Error opening employee database");
        send(client_socket, "Server error. Please try again later.\n", strlen("Server error. Please try again later.\n"), 0);
        return;
    }

    FILE *file = fdopen(file_fd, "r");
    if (!file) {
        perror("fdopen failed");
        close(file_fd);
        send(client_socket, "Server error. Please try again later.\n", strlen("Server error. Please try again later.\n"), 0);
        return;
    }

    // Step 1: Display the list of employees
    send(client_socket, "List of employees available for feedback:\n", strlen("List of employees available for feedback:\n"), 0);

    while (fgets(line, sizeof(line), file)) {
        // Assuming employee lines are in the format: emp_id username password
        int parsed = sscanf(line, "%s %s", emp_id, emp_username);
        if (parsed < 2) continue; // Invalid line format, skip

        // Send employee info to the customer
        snprintf(buffer, sizeof(buffer), "Employee ID: %s, Username: %s\n", emp_id, emp_username);
        send(client_socket, buffer, strlen(buffer), 0);
    }
    fclose(file); // Close employee file

    // Step 2: Ask the customer to select an employee for feedback
    send(client_socket, "Enter the Employee ID you want to give feedback to: ", strlen("Enter the Employee ID you want to give feedback to: "), 0);
    recv(client_socket, selected_emp_id, sizeof(selected_emp_id), 0);
    trim_newline_and_control_chars(selected_emp_id); // Clean input

    // Step 3: Ask the customer to provide feedback
    send(client_socket, "Enter your feedback: ", strlen("Enter your feedback: "), 0);
    recv(client_socket, feedback, sizeof(feedback), 0);
    trim_newline_and_control_chars(feedback); // Clean input

    // Step 4: Store the feedback in the feedback file
    file_fd = open(feedback_file, O_WRONLY | O_APPEND | O_CREAT, 0644);
    if (file_fd < 0) {
        perror("Error opening feedback file");
        send(client_socket, "Server error. Please try again later.\n", strlen("Server error. Please try again later.\n"), 0);
        return;
    }

    FILE *feedback_fp = fdopen(file_fd, "a");
    if (!feedback_fp) {
        perror("fdopen failed");
        close(file_fd);
        send(client_socket, "Server error. Please try again later.\n", strlen("Server error. Please try again later.\n"), 0);
        return;
    }

    // Write feedback to the file (emp_id: feedback)
    fprintf(feedback_fp, "Employee ID: %s, Feedback: %s\n", selected_emp_id, feedback);

    fclose(feedback_fp); // Close feedback file

    send(client_socket, "Feedback submitted successfully!\n", strlen("Feedback submitted successfully!\n"), 0);
}

void log_transaction(int cust_id, const char *transaction_type, double amount) {
    FILE *file = fopen("transactions_db.txt", "a");
    if (!file) {
        perror("Error opening transaction log file");
        return;
    }

    // Get current timestamp
    time_t now = time(NULL);
    struct tm *t = localtime(&now);
    char timestamp[20];
    strftime(timestamp, sizeof(timestamp), "%Y-%m-%d %H:%M:%S", t);

    // Write transaction log entry
    fprintf(file, "%d, %s, %.2lf, %s\n", cust_id, transaction_type, amount, timestamp);

    fclose(file);
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
		"7. Check your loan status \n"
		"8. View Transaction History\n"
		"9. Provide Feedback\n"
                "Logout\n"
		"Exit\n"
                "Enter your choice: ");
        send(client_socket, buffer, strlen(buffer), 0);
        memset(buffer, 0, BUFFER_SIZE);
        recv(client_socket, buffer, sizeof(buffer), 0);
        choice = atoi(buffer);

        switch (choice) {
            case 1:
                // Change Password
                change_customer_password(client_socket);
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
                    			// Log the deposit transaction
                    			log_transaction(logged_in_cust_id, "deposit", deposit_amount);
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
                    			// Log the withdrawal transaction
                   	 		log_transaction(logged_in_cust_id, "withdraw", withdraw_amount);
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
		int customer_id;
		send(client_socket, "Please enter your customer ID: ", strlen("Please enter your customer ID: "), 0);
   		recv(client_socket, buffer, sizeof(buffer), 0);
    		customer_id = atoi(buffer);
		check_loan_status(client_socket, customer_id);
		break;
	    case 8:
		view_transaction_history(client_socket, logged_in_cust_id);
		break;
	    case 9:
		give_feedback_to_employee(client_socket, "employee_db.txt", "employee_feedback.txt");
		break;
            case 10:
                send(client_socket, "Logged out.\n", strlen("Logged out.\n"), 0);
                break;
            default:
                send(client_socket, "Invalid choice! Please try again.\n", strlen("Invalid choice! Please try again.\n"), 0);
                break;
        }
    } while (choice != 10);
}
// Function to get the next employee ID in the format emp_1, emp_2, etc.
int get_next_employee_id_num() {
    FILE *file = fopen(EMPLOYEE_DB, "r");
    if (!file) {
        // If file doesn't exist, start with ID 1
        return 1;
    }

    char line[BUFFER_SIZE];
    int last_id = 0;
    int current_id;
    char emp_id_str[10];

    while (fgets(line, sizeof(line), file)) {
        if (sscanf(line, "%s", emp_id_str) == 1) {
            if (sscanf(emp_id_str, "emp_%d", &current_id) == 1) {
                if (current_id > last_id) {
                    last_id = current_id;
                }
            }
        }
    }
    fclose(file);
    return last_id + 1;
}
// Function to add a new employee with a unique emp_id
int add_new_employee(int client_socket) {
    char username[50], password[50];
    char emp_id[10];
    int next_id;
    char buffer[BUFFER_SIZE];

    // Get the next employee ID
    next_id = get_next_employee_id_num();
    snprintf(emp_id, sizeof(emp_id), "emp_%d", next_id);

    // Prompt for username
    send(client_socket, "Enter new Employee Username: ", strlen("Enter new Employee Username: "), 0);
    recv(client_socket, username, sizeof(username), 0);
    trim_newline(username); // Clean input

    // Prompt for password
    send(client_socket, "Enter new Employee Password: ", strlen("Enter new Employee Password: "), 0);
    recv(client_socket, password, sizeof(password), 0);
    trim_newline(password); // Clean input

    // Open employee_db.txt in append mode
    FILE *file = fopen(EMPLOYEE_DB, "a");
    if (!file) {
        perror("Error opening employee database");
        send(client_socket, "Error opening employee database.\n", strlen("Error opening employee database.\n"), 0);
        return 0;
    }

    // Write the new employee record: emp_id username password
    fprintf(file, "%s %s %s\n", emp_id, username, password);
    fclose(file);

    // Notify the manager of successful addition
    snprintf(buffer, sizeof(buffer), "Employee added successfully! Employee ID: %s\n", emp_id);
    send(client_socket, buffer, strlen(buffer), 0);

    return 1;
}
int view_assigned_loans(int client_socket) {
    char buffer[BUFFER_SIZE];

    if (strlen(logged_in_emp_id) == 0) {
        send(client_socket, "Error: No employee is logged in.\n", strlen("Error: No employee is logged in.\n"), 0);
        return 0;
    }

    FILE *assignment_file = fopen(LOAN_ASSIGNMENTS_FILE, "r");
    if (!assignment_file) {
        perror("Error opening loan assignments file");
        send(client_socket, "Error opening loan assignments file.\n", 
             strlen("Error opening loan assignments file.\n"), 0);
        return 0;
    }

    // Collect all loan_ids and statuses assigned to this employee
    int loan_ids[100];  // Assuming a maximum of 100 loans per employee
    char loan_status[100][20];  // To hold the status for each loan
    int count = 0;
    char line[BUFFER_SIZE];

    while (fgets(line, sizeof(line), assignment_file)) {
        char temp_loan_id[BUFFER_SIZE], temp_employee_id[BUFFER_SIZE], temp_status[20];
        sscanf(line, "%s %s %s", temp_loan_id, temp_employee_id, temp_status);
        
        // Check if the current employee ID matches the logged in employee ID
        if (strcmp(temp_employee_id, logged_in_emp_id) == 0) {
            // Add loan_id to the list if it matches and store the status
            loan_ids[count] = atoi(temp_loan_id + 5);  // Get the ID from "loan_x" format
            strncpy(loan_status[count], temp_status, sizeof(temp_status) - 1);
            loan_status[count][sizeof(temp_status) - 1] = '\0';  // Ensure null-termination
            count++;
            if (count >= 100) break;  // Prevent overflow
        }
    }
    fclose(assignment_file);

    if (count == 0) {
        send(client_socket, "No loan applications assigned to you.\n", strlen("No loan applications assigned to you.\n"), 0);
        return 1;
    }

    // Open loans_db.txt to fetch loan details
    FILE *loan_db_file = fopen(LOAN_DB, "r");
    if (!loan_db_file) {
        perror("Error opening loans database file");
        send(client_socket, "Error opening loans database file.\n", strlen("Error opening loans database file.\n"), 0);
        return 0;
    }

    // Prepare the response
    char response[BUFFER_SIZE * 10] = "Your Assigned Loan Applications:\nLoan ID\tCustomer ID\tAmount\tStatus\n";
    for (int i = 0; i < count; i++) {
        rewind(loan_db_file);  // Reset file pointer to beginning
        int found = 0;
        char loan_line[BUFFER_SIZE];
        int cust_id;
        double amount;

        // Format loan_id as "loan_x"
        char loan_id_str[20];
        snprintf(loan_id_str, sizeof(loan_id_str), "loan_%d", loan_ids[i]);

        while (fgets(loan_line, sizeof(loan_line), loan_db_file)) {
            char current_loan_id[20];
            if (sscanf(loan_line, "%s %d %lf", current_loan_id, &cust_id, &amount) == 3) {
                if (strcmp(current_loan_id, loan_id_str) == 0) {
                    // Append loan details and status to response
                    char loan_info[BUFFER_SIZE];
                    snprintf(loan_info, sizeof(loan_info), "%s\t%d\t\t%.2lf\t%s\n", current_loan_id, cust_id, amount, loan_status[i]);
                    strncat(response, loan_info, sizeof(response) - strlen(response) - 1);
                    found = 1;
                    break;
                }
            }
        }

        if (!found) {
            // If loan details not found
            char error_msg[BUFFER_SIZE];
            snprintf(error_msg, sizeof(error_msg), "Loan ID %s details not found.\n", loan_id_str);
            strncat(response, error_msg, sizeof(response) - strlen(response) - 1);
        }
    }

    fclose(loan_db_file);

    // Send the response to the employee
    send(client_socket, response, strlen(response), 0);

    return 1;
}
int change_employee_password(int client_socket) {
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
    if (update_employee_password(logged_in_emp_id, new_password)) {
        send(client_socket, "Password changed successfully!\n", strlen("Password changed successfully!\n"), 0);
        return 1;
    } else {
        send(client_socket, "Error changing password. Please try again.\n", strlen("Error changing password. Please try again.\n"), 0);
        return 0;
    }
}

int update_employee_password(const char* emp_id, const char* new_password) {
    FILE *file = fopen(EMPLOYEE_DB, "r");
    if (!file) {
        perror("Error opening employee database for reading");
        return 0;
    }

    // Open a temporary file for writing
    FILE *temp = fopen("employee_db.tmp", "w");
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
        char current_emp_id[50], username[50], password[50];
        sscanf(line, "%s %s %s", current_emp_id, username, password);
        
        if (strcmp(current_emp_id, emp_id) == 0) {
            // Replace the password
            fprintf(temp, "%s %s %s\n", current_emp_id, username, new_password);
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
        remove("employee_db.tmp");
        return 0;
    }

    // Replace the original file with the temporary file
    if (rename("employee_db.tmp", EMPLOYEE_DB) != 0) {
        perror("Error renaming temporary file");
        return 0;
    }

    return 1;
}

void view_customer_transaction_history(int client_socket, int cust_id) {
    FILE *file = fopen("transactions_db.txt", "r");
    if (!file) {
        perror("Error opening transaction log file");
        send(client_socket, "Error retrieving transaction history.\n", strlen("Error retrieving transaction history.\n"), 0);
        return;
    }

    char line[BUFFER_SIZE];
    char response[BUFFER_SIZE * 10] = "Customer Transaction History (Passbook):\n";
    int has_transactions = 0;

    while (fgets(line, sizeof(line), file)) {
        int current_id;
        char transaction_type[50];
        double amount;
        char timestamp[50];

        // Extracting transaction details from the log
        sscanf(line, "%d, %49[^,], %lf, %49[^\n]", &current_id, transaction_type, &amount, timestamp);

        // Check if the transaction belongs to the customer
        if (current_id == cust_id) {
            has_transactions = 1;
            char transaction_details[BUFFER_SIZE];
            snprintf(transaction_details, sizeof(transaction_details), "%s: %.2lf on %s\n", transaction_type, amount, timestamp);
            strcat(response, transaction_details);
        }
    }

    fclose(file);

    if (!has_transactions) {
        strcat(response, "No transactions found for this customer.\n");
    }

    // Send the transaction history to the employee
    send(client_socket, response, strlen(response), 0);
}


void employee_menu(int client_socket) {
    int choice;
   // char buffer[BUFFER_SIZE];
 char buffer[BUFFER_SIZE];


    do {
        // Display employee menu
        snprintf(buffer, sizeof(buffer),
                "\n===== Employee Menu =====\n"
                "1. View Assigned Loan Applications\n"
		"2. Approve/ Reject Loans\n"
		"3. View Customer Transactions\n"
                "4. Change Password\n"
                "Logout\n"
		"Exit\n"
                "Enter your choice: ");
        send(client_socket, buffer, strlen(buffer), 0);
        memset(buffer, 0, BUFFER_SIZE);
        recv(client_socket, buffer, sizeof(buffer), 0);
        choice = atoi(buffer);

        switch (choice) {
            case 1:
                // View Assigned Loan Applications
                if (!view_assigned_loans(client_socket)) {
                    // Message already sent within view_assigned_loans()
                }
                break;
	    case 2:
		approve_or_reject_loan(client_socket);
		break;
	    case 3:
		{
        char cust_id_buffer[BUFFER_SIZE];
        int target_cust_id;

        // Ask for the customer ID
        send(client_socket, "Enter the Customer ID to view transactions: ", strlen("Enter the Customer ID to view transactions: "), 0);
        recv(client_socket, cust_id_buffer, sizeof(cust_id_buffer), 0);
        target_cust_id = atoi(cust_id_buffer);

        if (target_cust_id <= 0) {
            send(client_socket, "Invalid Customer ID.\n", strlen("Invalid Customer ID.\n"), 0);
        } else if (!customer_exists(target_cust_id)) {
            send(client_socket, "Customer ID does not exist.\n", strlen("Customer ID does not exist.\n"), 0);
        } else {
            // Display the customer's transaction history (passbook)
            view_customer_transaction_history(client_socket, target_cust_id);
        }
    }
    break;
            case 4:
                change_employee_password(client_socket);
                break;
            case 5:
                // Logout
                send(client_socket, "Logged out.\n", strlen("Logged out.\n"), 0);
                break;
            default:
                send(client_socket, "Invalid choice! Please try again.\n", 
                     strlen("Invalid choice! Please try again.\n"), 0);
                break;
        }
    } while (choice != 5);
}
void approve_or_reject_loan(int client_socket) {
    FILE *file = fopen(LOAN_ASSIGNMENTS_FILE, "r+");
    char buffer[BUFFER_SIZE];
    char loan_id[50], status[20];
    int choice;

    if (!file) {
        send(client_socket, "Error opening loan assignments file.\n", strlen("Error opening loan assignments file.\n"), 0);
        return;
    }

    // Show the assigned loans first
    view_assigned_loans(client_socket);

    // Ask employee to enter Loan ID
    send(client_socket, "Enter Loan ID to approve/reject: ", strlen("Enter Loan ID to approve/reject: "), 0);
    recv(client_socket, loan_id, sizeof(loan_id), 0);
    strtok(loan_id, "\n");

    // Ask for approval or rejection
    send(client_socket, "Enter 1 to Approve, 2 to Reject: ", strlen("Enter 1 to Approve, 2 to Reject: "), 0);
    recv(client_socket, buffer, sizeof(buffer), 0);
    choice = atoi(buffer);

    if (choice == 1) {
        strcpy(status, "Approved");
    } else if (choice == 2) {
        strcpy(status, "Rejected");
    } else {
        send(client_socket, "Invalid choice!\n", strlen("Invalid choice!\n"), 0);
        fclose(file);
        return;
    }

    // Now update the loan status in the file
    char line[BUFFER_SIZE];
    char updated_loans[BUFFER_SIZE * 100] = "";
    int loan_found = 0;

    while (fgets(line, sizeof(line), file)) {
        char temp_loan_id[50], temp_employee_id[50], temp_status[20];
        sscanf(line, "%s %s %s", temp_loan_id, temp_employee_id, temp_status);

        // Update status if loan_id matches
        if (strcmp(temp_loan_id, loan_id) == 0) {
            loan_found = 1;
            snprintf(line, sizeof(line), "%s %s %s\n", temp_loan_id, temp_employee_id, status);
        }

        // Append the line to updated loans
        strncat(updated_loans, line, sizeof(updated_loans) - strlen(updated_loans) - 1);
    }

    // Write back the updated loans to the file
    freopen(LOAN_ASSIGNMENTS_FILE, "w", file);
    fputs(updated_loans, file);
    fclose(file);

    if (loan_found) {
        send(client_socket, "Loan status updated successfully.\n", strlen("Loan status updated successfully.\n"), 0);
    } else {
        send(client_socket, "Loan ID not found.\n", strlen("Loan ID not found.\n"), 0);
    }
}

void view_employee_feedback(int client_socket, const char *feedback_file) {
    char line[BUFFER_SIZE];
    int file_fd;

    // Open feedback file
    file_fd = open(feedback_file, O_RDONLY);
    if (file_fd < 0) {
        perror("Error opening feedback file");
        send(client_socket, "No feedback available at the moment.\n", strlen("No feedback available at the moment.\n"), 0);
        return;
    }

    FILE *file = fdopen(file_fd, "r");
    if (!file) {
        perror("fdopen failed");
        close(file_fd);
        send(client_socket, "No feedback available at the moment.\n", strlen("No feedback available at the moment.\n"), 0);
        return;
    }

    send(client_socket, "Employee Feedbacks:\n", strlen("Employee Feedbacks:\n"), 0);

    // Read and send each line of feedback to the manager
    while (fgets(line, sizeof(line), file)) {
        send(client_socket, line, strlen(line), 0);
    }

    fclose(file); // Close feedback file
}

// Function to implement manager menu
void manager_menu(int client_socket) {
    int choice, cust_id, loan_id, employee_id;
    char buffer[BUFFER_SIZE];

    do {
        // Display manager menu
        snprintf(buffer, sizeof(buffer),
                "\n===== Manager Menu =====\n"
                "1. Activate Customer Account\n"
                "2. Deactivate Customer Account\n"
                "3. Assign Loan Application to Employee\n"
		"4. View Employee Feedbacks\n"
                "Logout\n"
		"Exit\n"
                "Enter your choice: ");
        send(client_socket, buffer, strlen(buffer), 0);
        memset(buffer, 0, BUFFER_SIZE);
        recv(client_socket, buffer, sizeof(buffer), 0);
        choice = atoi(buffer);

        switch (choice) {
            case 1:
                // Activate customer account
                send(client_socket, "Enter Customer ID to activate: ", 
                     strlen("Enter Customer ID to activate: "), 0);
                recv(client_socket, buffer, sizeof(buffer), 0);
                cust_id = atoi(buffer);
                if (cust_id <= 0) {
                    send(client_socket, "Invalid Customer ID.\n", strlen("Invalid Customer ID.\n"), 0);
                } else {
                    update_customer_status_func(client_socket, cust_id, 1); // 1 = active
                }
                break;

            case 2:
                // Deactivate customer account
                send(client_socket, "Enter Customer ID to deactivate: ", 
                     strlen("Enter Customer ID to deactivate: "), 0);
                recv(client_socket, buffer, sizeof(buffer), 0);
                cust_id = atoi(buffer);
                if (cust_id <= 0) {
                    send(client_socket, "Invalid Customer ID.\n", strlen("Invalid Customer ID.\n"), 0);
                } else {
                    update_customer_status_func(client_socket, cust_id, 0); // 0 = inactive
                }
                break;

            case 3:
              
		int loan_id;
    		char employee_id[BUFFER_SIZE];
                char formatted_emp_id[BUFFER_SIZE]; 
		#define EMP_ID_MAX_LEN 40  
		
		// Ask manager for loan ID
		send(client_socket, "Enter Loan ID: ", strlen("Enter Loan ID: "), 0);
		recv(client_socket, buffer, sizeof(buffer), 0);
		loan_id = atoi(buffer); 

		// Ask manager for employee ID
		send(client_socket, "Enter Employee ID: ", strlen("Enter Employee ID: "), 0);
		recv(client_socket, employee_id, sizeof(employee_id), 0);
		strtok(employee_id, "\n");  // Remove any trailing newline

		// Ensure the length of employee_id doesn't exceed the allowed maximum
		if (strlen(employee_id) > EMP_ID_MAX_LEN) {
    		send(client_socket, "Employee ID too long. Maximum allowed length is 40 characters.\n", BUFFER_SIZE, 0);
   		 break;  // Exit the case or handle it as necessary
		}

		// Safely format employee ID
		snprintf(formatted_emp_id, sizeof(formatted_emp_id), "emp_%.*s", EMP_ID_MAX_LEN, employee_id);

		// Call the assign_loan_to_employee_func to assign loan to employee
		assign_loan_to_employee_func(client_socket, loan_id, formatted_emp_id);
		break;

	    case 4:
		view_employee_feedback(client_socket, "employee_feedback.txt");
		break;
	    case 5:
                // Logout
                send(client_socket, "Logged out.\n", strlen("Logged out.\n"), 0);
                break;

            default:
                send(client_socket, "Invalid choice! Please try again.\n", 
                     strlen("Invalid choice! Please try again.\n"), 0);
                break;
        }
    } while (choice != 5);
}

// Function to implement manager menu
int update_customer_status_func(int client_socket, int cust_id, int new_status) {
    FILE *status_file;
    char line[BUFFER_SIZE];
    char all_statuses[BUFFER_SIZE * 100] = "";  // Buffer to hold all statuses
    int found = 0;

    // Open customer_status file
    status_file = fopen(CUSTOMER_STATUS_FILE, "r+");
    if (status_file == NULL) {
        perror("Error opening customer status file");
        send(client_socket, "Error opening customer status file.\n",
             strlen("Error opening customer status file.\n"), 0);
        return 0;
    }

    // Read through the file and search for the customer
    while (fgets(line, sizeof(line), status_file) != NULL) {
        int temp_cust_id, temp_status;
        sscanf(line, "%d %d", &temp_cust_id, &temp_status);

        // If the customer is found, update the status
        if (temp_cust_id == cust_id) {
            found = 1;
            snprintf(line, sizeof(line), "%d %d\n", cust_id, new_status); // Update status
        }

        // Add the (possibly updated) line to all_statuses buffer
        strncat(all_statuses, line, sizeof(all_statuses) - strlen(all_statuses) - 1);
    }

    if (!found) {
        // If customer not found, add a new entry
        snprintf(line, sizeof(line), "%d %d\n", cust_id, new_status);
        strncat(all_statuses, line, sizeof(all_statuses) - strlen(all_statuses) - 1);
    }

    // Rewind file and write all statuses back
    freopen(CUSTOMER_STATUS_FILE, "w", status_file);
    fputs(all_statuses, status_file);
    fclose(status_file);

    // Notify manager of the status update
    if (new_status == 1) {
        send(client_socket, "Customer account activated.\n", strlen("Customer account activated.\n"), 0);
    } else {
        send(client_socket, "Customer account deactivated.\n", strlen("Customer account deactivated.\n"), 0);
    }

    return 1;
}

int assign_loan_to_employee_func(int client_socket, int loan_id, const char *employee_id) {
    FILE *assignment_file;
    char line[BUFFER_SIZE];
    char all_assignments[BUFFER_SIZE * 100] = "";  // Buffer to hold all assignments
    int found = 0;

    char formatted_loan_id[50];  // Buffer to store the formatted loan ID
    char formatted_line[BUFFER_SIZE];  // Buffer to store the formatted line for file output
    snprintf(formatted_loan_id, sizeof(formatted_loan_id), "loan_%d", loan_id);  // Format loan ID

    // Open loan_assignments file
    assignment_file = fopen(LOAN_ASSIGNMENTS_FILE, "r+");
    if (assignment_file == NULL) {
        perror("Error opening loan assignment file");
        send(client_socket, "Error opening loan assignment file.\n", strlen("Error opening loan assignment file.\n"), 0);
        return 0;
    }

    // Read through the file and check for existing assignment for the loan
    while (fgets(line, sizeof(line), assignment_file) != NULL) {
        char temp_loan_id[50], temp_employee_id[50], temp_status[20];
        sscanf(line, "%s %s %s", temp_loan_id, temp_employee_id, temp_status);

        // If loan_id is found, update the employee_id but keep the status
        if (strcmp(temp_loan_id, formatted_loan_id) == 0) {
            found = 1;
            // Keep the existing status and update the employee ID
            snprintf(formatted_line, sizeof(formatted_line), "%s %s %s\n", formatted_loan_id, employee_id, temp_status);
            strncpy(line, formatted_line, sizeof(line) - 1);  // Update the line buffer
            line[sizeof(line) - 1] = '\0';  // Ensure null-termination
        }

        // Add the (possibly updated) line to all_assignments buffer
        strncat(all_assignments, line, sizeof(all_assignments) - strlen(all_assignments) - 1);
    }

    if (!found) {
        // If loan assignment not found, add a new entry with "pending" status
        snprintf(formatted_line, sizeof(formatted_line), "%s %s pending\n", formatted_loan_id, employee_id);
        strncat(all_assignments, formatted_line, sizeof(all_assignments) - strlen(all_assignments) - 1);
    }

    // Rewind file and write all assignments back
    freopen(LOAN_ASSIGNMENTS_FILE, "w", assignment_file);
    fputs(all_assignments, assignment_file);
    fclose(assignment_file);

    // Notify manager of the assignment update
    send(client_socket, "Loan assigned to employee successfully with 'pending' status.\n",
         strlen("Loan assigned to employee successfully with 'pending' status.\n"), 0);

    return 1;
}


// Function to implement admin menu
void admin_menu(int client_socket) {
     int choice, cust_id, loan_id, employee_id;
    char buffer[BUFFER_SIZE];

    do {
        // Display manager menu
        snprintf(buffer, sizeof(buffer),
                "\n===== Admin Menu =====\n"
                "1. Add New Employee\n"
                "Logout\n"
		"Exit\n"
                "Enter your choice: ");
        send(client_socket, buffer, strlen(buffer), 0);
        memset(buffer, 0, BUFFER_SIZE);
        recv(client_socket, buffer, sizeof(buffer), 0);
        choice = atoi(buffer);

        switch (choice) {
            case 1:
                // Add New Employee
                add_new_employee(client_socket);
                break;

            case 2:
                // Logout
                send(client_socket, "Logged out.\n", strlen("Logged out.\n"), 0);
                break;

            default:
                send(client_socket, "Invalid choice! Please try again.\n", 
                     strlen("Invalid choice! Please try again.\n"), 0);
                break;
        }
    } while (choice != 2);
}
