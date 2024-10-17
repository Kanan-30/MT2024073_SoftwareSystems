#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <sys/file.h> 
#include <time.h>
#include <sys/types.h>
#include <errno.h>
#include <ctype.h>


#define PORT 5081
#define MAX_CLIENTS 10
#define BUFFER_SIZE 1048
#define USERNAME_SIZE 50
#define PASSWORD_SIZE 50
#define CUSTOMER_DB "customer.db"
#define EMPLOYEE_DB "employee_db.txt"
#define MANAGER_DB "manager_db.txt"
#define ADMIN_DB "admin_db.txt"
#define LOAN_DB "loan_db.txt"
#define CUSTOMER_STATUS_FILE "customer_status.txt"
#define LOAN_ASSIGNMENTS_FILE "loan_assignments.txt"
#define FEEDBACK_FILE "employee_feedback.txt" 

typedef struct {
    char cust_id[10];   
    char username[50];
    char password[50];
    char amount[20];    
} Customer;

char logged_in_username[50];       
int logged_in_cust_id = -1;        
double logged_in_amount = 000.00;   
char logged_in_emp_id[10];  
char logged_in_emp_username[50];  

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
int admin_login(int client_socket);
int manager_login(int client_socket);
int employee_login(int client_socket);

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
             if (employee_login(client_socket)) {
            employee_menu(client_socket);
             }
           break;

        case 3:
            if (manager_login(client_socket)) {
            manager_menu(client_socket);
        }
            break;
        case 4:
            if (admin_login(client_socket)) {
                admin_menu(client_socket);
            }
            break;
        default:
            send(client_socket, "Invalid User Type!\n", strlen("Invalid User Type!\n"), 0);
            break;
    }
}


void trim_newline(char *str) {
    int len = strlen(str);
    if (len > 0 && (str[len-1] == '\n' || str[len-1] == '\r')) {
        str[len-1] = '\0';
    }
}

// Function to trim newline and control characters
void trim_newline_and_control_chars(char *str) {
    char *end;

    // Remove leading control characters (whitespace, newlines, etc.)
    while (isspace((unsigned char)*str)) {
        str++;
    }

    // Remove trailing newline or control characters
    end = str + strlen(str) - 1;
    while (end > str && iscntrl((unsigned char)*end)) {
        end--;
    }
    *(end + 1) = '\0';  // Null-terminate after trimming
}

// Function to read a line from a file descriptor
ssize_t read_line(int fd, char *buffer, size_t size) {
    ssize_t bytes_read = 0;
    char ch;

    while (bytes_read < size - 1) {
        ssize_t n = read(fd, &ch, 1);
        if (n < 0) {
            perror("read failed");
            return -1;
        } else if (n == 0) {
            break;  // EOF reached
        }

        // If it's a newline character, stop reading
        if (ch == '\n') {
            break;
        }

        buffer[bytes_read++] = ch;
    }

    buffer[bytes_read] = '\0';  // Null-terminate the string
    return bytes_read;
}

int get_next_customer_id() {
    int fd = open(CUSTOMER_DB, O_RDONLY);
    if (fd < 0) {
        // If file doesn't exist, start with ID "1"
        return 1;
    }

    Customer last_customer;
    int last_id = 0;
    while (read(fd, &last_customer, sizeof(Customer)) > 0) {
        int current_id = atoi(last_customer.cust_id);
        if (current_id > last_id) {
            last_id = current_id;
        }
    }
    close(fd);

    // Return the next available ID as an integer
    return last_id + 1;
}
int customer_exists(int cust_id) {
    int fd = open(CUSTOMER_DB, O_RDONLY);
    if (fd < 0) {
        return 0; // Customer not found if file doesn't exist
    }

    Customer customer;
    char cust_id_str[10];
    snprintf(cust_id_str, sizeof(cust_id_str), "%d", cust_id);

    while (read(fd, &customer, sizeof(Customer)) > 0) {
        if (strcmp(customer.cust_id, cust_id_str) == 0) {
            close(fd);
            return 1; // Customer found
        }
    }
    close(fd);
    return 0; // Customer not found
}
double get_customer_amount(int cust_id) {
    int fd = open(CUSTOMER_DB, O_RDONLY);
    if (fd < 0) {
        perror("Error opening customer database for reading amount");
        return -1.0;
    }

    Customer customer;
    char cust_id_str[10];
    snprintf(cust_id_str, sizeof(cust_id_str), "%d", cust_id);

    while (read(fd, &customer, sizeof(Customer)) > 0) {
        if (strcmp(customer.cust_id, cust_id_str) == 0) {
            close(fd);
            return atof(customer.amount); // Convert string to double
        }
    }
    close(fd);
    return -1.0; // Customer not found
}

int update_customer_amount(int cust_id, double new_amount) {
    int fd = open(CUSTOMER_DB, O_RDWR);
    if (fd < 0) {
        perror("Error opening customer database for updating");
        return 0;
    }

    // Lock the file before reading and updating
    if (flock(fd, LOCK_EX) < 0) {
        perror("Error locking the file");
        close(fd);
        return 0;
    }

    Customer customer;
    off_t pos;
    char cust_id_str[10];
    snprintf(cust_id_str, sizeof(cust_id_str), "%d", cust_id);

    // Move the file pointer to the start of the file
    if (lseek(fd, 0, SEEK_SET) < 0) {
        perror("Error seeking to the start of the file");
        flock(fd, LOCK_UN);
        close(fd);
        return 0;
    }

    // Loop through the file to find the customer
    while ((pos = lseek(fd, 0, SEEK_CUR)) >= 0 && read(fd, &customer, sizeof(Customer)) == sizeof(Customer)) {
        if (strcmp(customer.cust_id, cust_id_str) == 0) {
            // Update the customer's amount
            snprintf(customer.amount, sizeof(customer.amount), "%.2f", new_amount);

            // Move file pointer to the position where the customer was found
            if (lseek(fd, pos, SEEK_SET) < 0) {
                perror("Error seeking to update position");
                flock(fd, LOCK_UN);
                close(fd);
                return 0;
            }

            // Write the updated customer information back to the file
            if (write(fd, &customer, sizeof(Customer)) != sizeof(Customer)) {
                perror("Error writing updated customer information");
                flock(fd, LOCK_UN);
                close(fd);
                return 0;
            }

            // Ensure data is written to disk
            fsync(fd);

            // Unlock and close the file
            flock(fd, LOCK_UN);
            close(fd);
            return 1; // Update successful
        }
    }

    // Unlock and close the file if customer not found
    flock(fd, LOCK_UN);
    close(fd);
    return 0; // Customer not found
}


int create_new_account(int client_socket, const char *role_file, const char *role) {
    char username[50], password[50];
    memset(username, 0, sizeof(username));
    memset(password, 0, sizeof(password));

    send(client_socket, "Enter new Username: ", strlen("Enter new Username: "), 0);
    recv(client_socket, username, sizeof(username), 0);
    trim_newline(username);

    send(client_socket, "Enter new Password: ", strlen("Enter new Password: "), 0);
    recv(client_socket, password, sizeof(password), 0);
    trim_newline(password);

    int cust_id = get_next_customer_id();

    Customer new_customer;
    snprintf(new_customer.cust_id, sizeof(new_customer.cust_id), "%d", cust_id); // Store cust_id as a string
    strncpy(new_customer.username, username, sizeof(new_customer.username) - 1);
    strncpy(new_customer.password, password, sizeof(new_customer.password) - 1);
    snprintf(new_customer.amount, sizeof(new_customer.amount), "%.2f", 0.00); // Store initial amount as a string

    int fd = open(CUSTOMER_DB, O_WRONLY | O_CREAT | O_APPEND, 0644);
    if (fd < 0) {
        perror("Error opening customer file");
        send(client_socket, "Server error. Please try again later.\n", strlen("Server error. Please try again later.\n"), 0);
        return 0;
    }
    write(fd, &new_customer, sizeof(Customer));
    close(fd);

    char success_msg[BUFFER_SIZE];
    snprintf(success_msg, sizeof(success_msg), "Account created successfully! Your Customer ID is %d.\n", cust_id);
    send(client_socket, success_msg, strlen(success_msg), 0);
    return 1;
}


int login_user(int client_socket, const char *role_file, const char *role) {
    char buffer[BUFFER_SIZE], username[50], password[50];
    int file_fd;

    memset(buffer, 0, BUFFER_SIZE);        // Clear the buffer
    memset(username, 0, sizeof(username)); // Clear username
    memset(password, 0, sizeof(password)); // Clear password

    // Ask for username
    send(client_socket, "Enter Username: ", strlen("Enter Username: "), 0);
    recv(client_socket, username, sizeof(username), 0);
    trim_newline(username); // Clean input

    // Open the role file (e.g., customer file)
    file_fd = open(role_file, O_RDONLY);
    if (file_fd < 0) {
        perror("Error opening file");
        send(client_socket, "Server error. Please try again later.\n",
             strlen("Server error. Please try again later.\n"), 0);
        return 0;
    }

    Customer customer;
    int found = 0;
    ssize_t read_bytes;

    // Read each customer entry and check if the username exists
    while ((read_bytes = read(file_fd, &customer, sizeof(Customer))) == sizeof(Customer)) {
        if (strcmp(customer.username, username) == 0) {
            found = 1; // Username found
            break;
        }
    }
    close(file_fd); // Close the file after reading

    if (found) {
        // Ask for password if username exists
        memset(password, 0, sizeof(password));
        send(client_socket, "Enter Password: ", strlen("Enter Password: "), 0);
        recv(client_socket, password, sizeof(password), 0);
        trim_newline(password); // Clean input

        if (strcmp(customer.password, password) == 0) {
            send(client_socket, "Login successful!\n", strlen("Login successful!\n"), 0);

            // **Pause here before showing the menu**
            const char *pause_message = "Press Enter to continue...\n";
            send(client_socket, pause_message, strlen(pause_message), 0);

            memset(buffer, 0, BUFFER_SIZE);
            recv(client_socket, buffer, BUFFER_SIZE, 0);  // Wait for client to press enter

            // Store the logged-in customer's data
            strcpy(logged_in_username, customer.username);
            logged_in_cust_id = atoi(customer.cust_id);
            logged_in_amount = atof(customer.amount);
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


//Transfer funds from customer 1 to customer 2
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
    int fd = open(CUSTOMER_DB, O_RDWR);
    if (fd < 0) {
        send(client_socket, "Error opening customer database for update.\n", strlen("Error opening customer database for update.\n"), 0);
        return 0;
    }

    // Lock the file for writing
    if (flock(fd, LOCK_EX) < 0) {
        send(client_socket, "Error locking the customer database.\n", strlen("Error locking the customer database.\n"), 0);
        close(fd);
        return 0;
    }

    // Update sender's balance
    if (!update_customer_amount(logged_in_cust_id, new_sender_balance)) {
        send(client_socket, "Error updating your account balance.\n", strlen("Error updating your account balance.\n"), 0);
        flock(fd, LOCK_UN);
        close(fd);
        return 0;
    }

    // Update receiver's balance
    if (!update_customer_amount(target_cust_id, new_receiver_balance)) {
        // Rollback sender's balance in case receiver update fails
        update_customer_amount(logged_in_cust_id, sender_balance);
        send(client_socket, "Error updating receiver's account balance. Transfer rolled back.\n", strlen("Error updating receiver's account balance. Transfer rolled back.\n"), 0);
        flock(fd, LOCK_UN);
        close(fd);
        return 0;
    }

    // Unlock and close the file
    flock(fd, LOCK_UN);
    close(fd);

    send(client_socket, "Transfer successful!\n", strlen("Transfer successful!\n"), 0);

    // Log the transfer transaction for both sender and receiver
    log_transaction(logged_in_cust_id, "transfer_sent", transfer_amount);
    log_transaction(target_cust_id, "transfer_received", transfer_amount);

    return 1;
}
void log_transaction(int cust_id, const char *transaction_type, double amount) {
    // Open transaction log file in append mode using system calls
    int file_fd = open("transactions_db.txt", O_WRONLY | O_APPEND | O_CREAT, 0644);
    if (file_fd < 0) {
        perror("Error opening transaction log file");
        return;
    }

    // Get current timestamp
    time_t now = time(NULL);
    struct tm *t = localtime(&now);
    char timestamp[20];
    strftime(timestamp, sizeof(timestamp), "%Y-%m-%d %H:%M:%S", t);

    // Prepare the transaction log entry
    char log_entry[BUFFER_SIZE];
    snprintf(log_entry, sizeof(log_entry), "%d, %s, %.2lf, %s\n", cust_id, transaction_type, amount, timestamp);

    // Write transaction log entry to file using system calls
    write(file_fd, log_entry, strlen(log_entry));

    // Close the file descriptor using system calls
    close(file_fd);
}
void view_transaction_history(int client_socket, int cust_id) {
    int file_fd = open("transactions_db.txt", O_RDONLY);
    if (file_fd < 0) {
        perror("Error opening transaction log file");
        send(client_socket, "Error retrieving transaction history.\n", strlen("Error retrieving transaction history.\n"), 0);
        return;
    }

    char line[BUFFER_SIZE];
    char response[BUFFER_SIZE * 10] = "Transaction History:\n";
    int has_transactions = 0;
    ssize_t bytes_read;

    // Read each line of the file using the custom read_line function
    while ((bytes_read = read_line(file_fd, line, sizeof(line))) > 0) {
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

    close(file_fd);

    if (!has_transactions) {
        strcat(response, "No transactions found.\n");
    }

    send(client_socket, response, strlen(response), 0);
}


int get_next_loan_id() {
    int loan_db_fd = open(LOAN_DB, O_RDONLY);  // Open loan database in read-only mode
    if (loan_db_fd < 0) {
        perror("Error opening loan database");
        return 1;  // Return 1 as the default loan ID if the file cannot be opened
    }

    char buffer[BUFFER_SIZE];
    ssize_t bytes_read;
    int max_loan_id = 0;
    int loan_id;

    // Read the file line by line
    while ((bytes_read = read_line(loan_db_fd, buffer, sizeof(buffer))) > 0) {
        // Parse the line for loan ID
        if (sscanf(buffer, "loan_%d", &loan_id) == 1) {
            if (loan_id > max_loan_id) {
                max_loan_id = loan_id;  // Track the highest loan ID
            }
        }
    }

    close(loan_db_fd);

    // Return the next loan ID, which is max_loan_id + 1
    return max_loan_id + 1;
}


int apply_for_loan(int client_socket) {
    char buffer[BUFFER_SIZE], loan_amount[20];
    char cust_id_str[10];
    int cust_id, loan_id;
    double balance;

    // Prompt the customer for their ID
    write(client_socket, "Enter your customer ID: ", strlen("Enter your customer ID: "));
    read(client_socket, buffer, BUFFER_SIZE);
    trim_newline(buffer);
    strncpy(cust_id_str, buffer, sizeof(cust_id_str) - 1);
    cust_id_str[sizeof(cust_id_str) - 1] = '\0';
    cust_id = atoi(cust_id_str);

    if (cust_id <= 0) {
        write(client_socket, "Invalid Customer ID.\n", strlen("Invalid Customer ID.\n"));
        return 0;
    }

    // Open customer_db file using system call
    int file_fd = open(CUSTOMER_DB, O_RDONLY);
    if (file_fd < 0) {
        perror("Error opening customer database");
        write(client_socket, "Error opening customer database.\n", strlen("Error opening customer database.\n"));
        return 0;
    }

    Customer customer;
    int found = 0;

    // Read and parse the customer_db file, assuming binary format
    while (read(file_fd, &customer, sizeof(Customer)) == sizeof(Customer)) {
        int temp_cust_id = atoi(customer.cust_id);
        double balance_val = atof(customer.amount);
        if (temp_cust_id == cust_id) {
            found = 1;
            balance = balance_val;
            break;
        }
    }

    if (!found) {
        write(client_socket, "Customer ID not found!\n", strlen("Customer ID not found!\n"));
        close(file_fd);
        return 0;
    }

    // Prompt the customer for the loan amount
    write(client_socket, "Enter loan amount: ", strlen("Enter loan amount: "));
    read(client_socket, loan_amount, sizeof(loan_amount));
    trim_newline(loan_amount);  // Clean input
    double loan_amt = atof(loan_amount);

    if (loan_amt <= 0) {
        write(client_socket, "Invalid loan amount.\n", strlen("Invalid loan amount.\n"));
        close(file_fd);
        return 0;
    }

    // Get the next loan ID from the loan database
    loan_id = get_next_loan_id();

    // Open loan_db to append the new loan
    int loan_db_fd = open(LOAN_DB, O_WRONLY | O_APPEND | O_CREAT, 0644);
    if (loan_db_fd < 0) {
        perror("Error opening loan database");
        write(client_socket, "Error opening loan database.\n", strlen("Error opening loan database.\n"));
        close(file_fd);
        return 0;
    }

    // Construct the loan record (loan_id, cust_id, loan_amount)
    snprintf(buffer, sizeof(buffer), "loan_%d %d %.2lf\n", loan_id, cust_id, loan_amt);

    // Write the loan record to the loan_db file
    if (write(loan_db_fd, buffer, strlen(buffer)) < 0) {
        perror("Error writing to loan database");
        write(client_socket, "Error writing to loan database.\n", strlen("Error writing to loan database.\n"));
        close(loan_db_fd);
        close(file_fd);
        return 0;
    }

    close(loan_db_fd);

    // Notify the customer that the loan was applied successfully
    write(client_socket, "Loan applied successfully!\n", strlen("Loan applied successfully!\n"));

    close(file_fd);
    return 1;
}
int check_loan_status(int client_socket, int customer_id) {
    int loan_db_fd = open(LOAN_DB, O_RDONLY);
    if (loan_db_fd < 0) {
        perror("Error opening loan database file");
        write(client_socket, "Error opening loan database file.\n", strlen("Error opening loan database file.\n"));
        return 0;
    }

    // Prepare response message for customer
    char response[BUFFER_SIZE * 10] = "Your Loan Status:\nLoan ID\tAmount\tStatus\n";

    // Buffer to read the file line by line
    char line[BUFFER_SIZE];
    int loan_found = 0;
    ssize_t bytes_read;

    // Read loan_db file using custom read_line function
    while ((bytes_read = read_line(loan_db_fd, line, sizeof(line))) > 0) {
        int loan_id, db_cust_id;
        double amount;

        // Parse loan_id, customer_id, and amount
        if (sscanf(line, "loan_%d %d %lf", &loan_id, &db_cust_id, &amount) == 3) {
            if (db_cust_id == customer_id) {
                loan_found = 1;

                // Open loan_assignments.txt using system calls
                int assignment_fd = open(LOAN_ASSIGNMENTS_FILE, O_RDONLY);
                if (assignment_fd < 0) {
                    perror("Error opening loan assignment file");
                    write(client_socket, "Error opening loan assignment file.\n", strlen("Error opening loan assignment file.\n"));
                    close(loan_db_fd);
                    return 0;
                }

                char assignment_line[BUFFER_SIZE];
                char status[20] = "Not Assigned";  // Default status if not found
                ssize_t assignment_bytes_read;

                // Read loan_assignments file using custom read_line function
                while ((assignment_bytes_read = read_line(assignment_fd, assignment_line, sizeof(assignment_line))) > 0) {
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

                close(assignment_fd);

                // Append loan information and status to the response
                char loan_info[BUFFER_SIZE];
                snprintf(loan_info, sizeof(loan_info), "loan_%d\t%.2lf\t%s\n", loan_id, amount, status);
                strncat(response, loan_info, sizeof(response) - strlen(response) - 1);
            }
        }
    }

    close(loan_db_fd);

    if (!loan_found) {
        write(client_socket, "No loans found for your account.\n", strlen("No loans found for your account.\n"));
    } else {
        // Send the response to the customer
        write(client_socket, response, strlen(response));
    }

    return 1;
}


int change_customer_password(int client_socket) {
    char buffer[BUFFER_SIZE];
    char new_password[50];
    char line[BUFFER_SIZE];

    // Prompt for the new password
    write(client_socket, "Enter your new password: ", strlen("Enter your new password: "));
    read(client_socket, new_password, sizeof(new_password));
    trim_newline(new_password); // Clean input

    // Validate new password
    if (strlen(new_password) == 0) {
        write(client_socket, "Password cannot be empty.\n", strlen("Password cannot be empty.\n"));
        return 0;
    }

    // Open the customer database file using system calls
    int file_fd = open(CUSTOMER_DB, O_RDONLY);
    if (file_fd < 0) {
        perror("Error opening customer database");
        write(client_socket, "Error opening customer database.\n", strlen("Error opening customer database.\n"));
        return 0;
    }

    // Open a temporary file to write the updated content
    int temp_fd = open("customer_db.tmp", O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if (temp_fd < 0) {
        perror("Error opening temporary file");
        close(file_fd);
        write(client_socket, "Error opening temporary file.\n", strlen("Error opening temporary file.\n"));
        return 0;
    }

    ssize_t bytes_read;
    int updated = 0;
    char file_cust_id_str[10], file_username[50], file_password[50];
    char amount_str[20];
    int cust_id;
    double balance;

    // Read through the file using a custom line reader (system calls only)
    while ((bytes_read = read_line(file_fd, line, sizeof(line))) > 0) {
        sscanf(line, "%s %s %s %s", file_cust_id_str, file_username, file_password, amount_str);
        cust_id = atoi(file_cust_id_str);
        balance = atof(amount_str);

        // If the username matches, update the password
        if (strcmp(file_username, logged_in_username) == 0) {
            // Update the password
            snprintf(buffer, sizeof(buffer), "%s %s %s %s\n", file_cust_id_str, file_username, new_password, amount_str);
            if (write(temp_fd, buffer, strlen(buffer)) < 0) {
                perror("Error writing to temporary file");
                write(client_socket, "Error updating password.\n", strlen("Error updating password.\n"));
                close(file_fd);
                close(temp_fd);
                return 0;
            }
            updated = 1;
        } else {
            // Write the line as-is
            snprintf(buffer, sizeof(buffer), "%s %s %s %s\n", file_cust_id_str, file_username, file_password, amount_str);
            if (write(temp_fd, buffer, strlen(buffer)) < 0) {
                perror("Error writing to temporary file");
                write(client_socket, "Error updating password.\n", strlen("Error updating password.\n"));
                close(file_fd);
                close(temp_fd);
                return 0;
            }
        }
    }

    close(file_fd);
    close(temp_fd);

    // If the password was updated, rename the temp file to replace the original
    if (updated) {
        if (rename("customer_db.tmp", CUSTOMER_DB) < 0) {
            perror("Error renaming temporary file");
            write(client_socket, "Error updating password.\n", strlen("Error updating password.\n"));
            return 0;
        }
        write(client_socket, "Password changed successfully!\n", strlen("Password changed successfully!\n"));
        return 1;
    } else {
        // Clean up the temporary file if no changes were made
        remove("customer_db.tmp");
        write(client_socket, "Error: Username not found.\n", strlen("Error: Username not found.\n"));
        return 0;
    }
}
void give_feedback_to_employee(int client_socket, const char *employee_db, const char *feedback_file) {
    char line[BUFFER_SIZE], emp_id[20], emp_username[50];
    char selected_emp_id[20], feedback[BUFFER_SIZE];
    char buffer[BUFFER_SIZE];
    int file_fd;
    ssize_t bytes_read;

    // Open employee_db to display list of employees
    file_fd = open(employee_db, O_RDONLY);
    if (file_fd < 0) {
        perror("Error opening employee database");
        write(client_socket, "Server error. Please try again later.\n", strlen("Server error. Please try again later.\n"));
        return;
    }

    // Step 1: Display the list of employees
    write(client_socket, "List of employees available for feedback:\n", strlen("List of employees available for feedback:\n"));

    while ((bytes_read = read_line(file_fd, line, sizeof(line))) > 0) {
        // Assuming employee lines are in the format: emp_id username password
        int parsed = sscanf(line, "%s %s", emp_id, emp_username);
        if (parsed < 2) continue; // Invalid line format, skip

        // Send employee info to the customer
        snprintf(buffer, sizeof(buffer), "Employee ID: %s, Username: %s\n", emp_id, emp_username);
        write(client_socket, buffer, strlen(buffer));
    }
    close(file_fd); // Close employee database file

    // Step 2: Ask the customer to select an employee for feedback
    write(client_socket, "Enter the Employee ID you want to give feedback to: ", strlen("Enter the Employee ID you want to give feedback to: "));
    read(client_socket, selected_emp_id, sizeof(selected_emp_id));
    trim_newline_and_control_chars(selected_emp_id); // Clean input

    // Step 3: Ask the customer to provide feedback
    write(client_socket, "Enter your feedback: ", strlen("Enter your feedback: "));
    read(client_socket, feedback, sizeof(feedback));
    trim_newline_and_control_chars(feedback); // Clean input

    // Step 4: Store the feedback in the feedback file
    file_fd = open(feedback_file, O_WRONLY | O_APPEND | O_CREAT, 0644);
    if (file_fd < 0) {
        perror("Error opening feedback file");
        write(client_socket, "Server error. Please try again later.\n", strlen("Server error. Please try again later.\n"));
        return;
    }

    // Construct feedback entry
    snprintf(buffer, sizeof(buffer), "Employee ID: %s, Feedback: %s\n", selected_emp_id, feedback);

    // Write feedback to the feedback file
    if (write(file_fd, buffer, strlen(buffer)) < 0) {
        perror("Error writing to feedback file");
        write(client_socket, "Error submitting feedback.\n", strlen("Error submitting feedback.\n"));
        close(file_fd);
        return;
    }

    close(file_fd); // Close feedback file

    // Notify the client
    write(client_socket, "Feedback submitted successfully!\n", strlen("Feedback submitted successfully!\n"));
}



void customer_menu(int client_socket) {
    int choice;
    char buffer[BUFFER_SIZE];

    do {
        // Display menu options to the customer
        snprintf(buffer, sizeof(buffer),
                 "\n===== Customer Menu =====\n"
                "1. Change Password\n"
                "2. View Account Balance\n"
                "3. Deposit Money\n"
                "4. Withdraw Money\n"
                "5. Transfer Funds\n"
                "6. Apply for a Loan\n"
                "7. Check Your Loan Status\n"
                "8. View Transaction History\n"
                "9. Provide Feedback\n"
                "logout\n"
		"exit\n"
                "Enter your choice: ");
        send(client_socket, buffer, strlen(buffer), 0);

        // Clear the buffer and receive user's choice
        memset(buffer, 0, BUFFER_SIZE);
        ssize_t recv_bytes = recv(client_socket, buffer, sizeof(buffer) - 1, 0);
        if (recv_bytes <= 0) {
            // Handle disconnection or error
            perror("recv failed or connection closed");
            break;
        }
        buffer[recv_bytes] = '\0';  // Ensure null-termination
        choice = atoi(buffer);

        switch (choice) {
            case 1:
                // Change Password
                if (!change_customer_password(client_socket)) {
                    send(client_socket, "Failed to change password.\n", strlen("Failed to change password.\n"), 0);
                }
                break;

            case 2:
                // View Account Balance
                {
                    double balance = get_customer_amount(logged_in_cust_id);
                    if (balance >= 0.0) {
                        snprintf(buffer, sizeof(buffer), "Your current account balance is: $%.2lf\n", balance);
                        send(client_socket, buffer, strlen(buffer), 0);
                    } else {
                        send(client_socket, "Error retrieving account balance.\n", strlen("Error retrieving account balance.\n"), 0);
                    }
                }
                break;

            case 3:  // Deposit Money
                {
                    double deposit_amount;
                    send(client_socket, "Enter amount to deposit: ", strlen("Enter amount to deposit: "), 0);
                    memset(buffer, 0, BUFFER_SIZE);
                    recv_bytes = recv(client_socket, buffer, sizeof(buffer) - 1, 0);
                    if (recv_bytes <= 0) {
                        perror("recv failed or connection closed");
                        break;
                    }
                    buffer[recv_bytes] = '\0';
                    trim_newline(buffer);
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
                                log_transaction(logged_in_cust_id, "deposit", deposit_amount);
                            } else {
                                send(client_socket, "Error updating account balance.\n", strlen("Error updating account balance.\n"), 0);
                            }
                        }
                    }
                }
                break;
	        case 4:  // Withdraw Money
               {
                    double withdraw_amount;
                    send(client_socket, "Enter amount to withdraw: ", strlen("Enter amount to withdraw: "), 0);
                    memset(buffer, 0, BUFFER_SIZE);
                    recv_bytes = recv(client_socket, buffer, sizeof(buffer) - 1, 0);
                    if (recv_bytes <= 0) {
                        perror("recv failed or connection closed");
                        break;
                    }
                    buffer[recv_bytes] = '\0';
                    trim_newline(buffer);
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
                                log_transaction(logged_in_cust_id, "withdraw", withdraw_amount);
                            } else {
                                send(client_socket, "Error updating account balance.\n", strlen("Error updating account balance.\n"), 0);
                            }
                        }
                    }
                }
                break;

            case 5:  // Transfer Funds
                if (!transfer_funds(client_socket)) {
                    // Transfer failed; message already sent within transfer_funds()
                }
                break;

            case 6:  // Apply for a Loan
                if (!apply_for_loan(client_socket)) {
                    // Message already sent within apply_for_loan()
                }
                break;

            case 7:  // Check Loan Status
                {
                    // Use logged_in_cust_id directly instead of asking for customer ID again
                    if (!check_loan_status(client_socket, logged_in_cust_id)) {
                        send(client_socket, "Failed to retrieve loan status.\n", strlen("Failed to retrieve loan status.\n"), 0);
                    }
                }
                break;

            case 8:  // View Transaction History
                view_transaction_history(client_socket, logged_in_cust_id);
                break;

            case 9:  // Provide Feedback
                give_feedback_to_employee(client_socket, "employee_db.txt", "employee_feedback.txt");
                break;

            case 10:  // Logout
                send(client_socket, "Logged out.\n", strlen("Logged out.\n"), 0);
                break;

            default:
                send(client_socket, "Invalid choice! Please try again.\n", strlen("Invalid choice! Please try again.\n"), 0);
                break;
        }
    } while (choice != 10);
}

// Function to implement the customer menu
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

    // Open employee_db.txt in append mode using system call
    int file_fd = open(EMPLOYEE_DB, O_WRONLY | O_APPEND | O_CREAT, 0644);
    if (file_fd < 0) {
        perror("Error opening employee database");
        send(client_socket, "Error opening employee database.\n", strlen("Error opening employee database.\n"), 0);
        return 0;
    }

    // Prepare the new employee record: emp_id username password
    char new_employee_record[BUFFER_SIZE];
    snprintf(new_employee_record, sizeof(new_employee_record), "%s %s %s\n", emp_id, username, password);

    // Write the new employee record to the file
    ssize_t bytes_written = write(file_fd, new_employee_record, strlen(new_employee_record));
    if (bytes_written < 0) {
        perror("Error writing to employee database");
        close(file_fd);
        send(client_socket, "Error writing to employee database.\n", strlen("Error writing to employee database.\n"), 0);
        return 0;
    }

    close(file_fd); // Close the file descriptor

    // Notify the manager of successful addition
    snprintf(buffer, sizeof(buffer), "Employee added successfully! Employee ID: %s\n", emp_id);
    send(client_socket, buffer, strlen(buffer), 0);

    return 1;
}


// view assigned loans
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

// CHange employee password
int change_employee_password(int client_socket) {
    char new_password[50];

    // Prompt for new password
    send(client_socket, "Enter your new password: ", strlen("Enter your new password: "), 0);
    
    // Receive new password
    ssize_t bytes_received = recv(client_socket, new_password, sizeof(new_password), 0);
    if (bytes_received <= 0) {
        send(client_socket, "Error receiving password.\n", strlen("Error receiving password.\n"), 0);
        return 0;
    }
    new_password[bytes_received] = '\0'; // Null-terminate the string
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

//Update password in file
int update_employee_password(const char* emp_id, const char* new_password) {
    int file_fd = open(EMPLOYEE_DB, O_RDONLY);
    if (file_fd < 0) {
        perror("Error opening employee database for reading");
        return 0;
    }

    // Create a temporary file for writing
    int temp_fd = open("employee_db.tmp", O_WRONLY | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR);
    if (temp_fd < 0) {
        perror("Error opening temporary file for writing");
        close(file_fd);
        return 0;
    }

    // Lock the original file for reading
    if (flock(file_fd, LOCK_SH) < 0) {
        perror("Error locking the original file");
        close(file_fd);
        close(temp_fd);
        return 0;
    }

    char line[BUFFER_SIZE];
    int updated = 0;

    // Read lines from the original file
    ssize_t bytes_read;
    while ((bytes_read = read(file_fd, line, sizeof(line) - 1)) > 0) {
        line[bytes_read] = '\0'; // Null-terminate the line
        char current_emp_id[50], username[50], password[50];
        sscanf(line, "%s %s %s", current_emp_id, username, password);
        
        if (strcmp(current_emp_id, emp_id) == 0) {
            // Replace the password
            dprintf(temp_fd, "%s %s %s\n", current_emp_id, username, new_password);
            updated = 1;
        } else {
            // Write the line as is
            write(temp_fd, line, strlen(line));
        }
    }

    // Unlock and close the original file
    flock(file_fd, LOCK_UN);
    close(file_fd);

    // Close the temporary file
    close(temp_fd);

    if (!updated) {
        // Remove the temporary file if no update was done
        unlink("employee_db.tmp");
        return 0;
    }

    // Replace the original file with the temporary file
    if (rename("employee_db.tmp", EMPLOYEE_DB) != 0) {
        perror("Error renaming temporary file");
        return 0;
    }

    return 1;
}

//Customer Transaction History
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
int employee_login(int client_socket) {
    char username[50], password[50];
    char buffer[BUFFER_SIZE];
    memset(username, 0, sizeof(username));
    memset(password, 0, sizeof(password));
    memset(buffer, 0, BUFFER_SIZE);

    // Prompt for employee username
    send(client_socket, "Enter Employee Username: ", strlen("Enter Employee Username: "), 0);
    recv(client_socket, username, sizeof(username), 0);
    trim_newline(username);

    // Prompt for employee password
    send(client_socket, "Enter Employee Password: ", strlen("Enter Employee Password: "), 0);
    recv(client_socket, password, sizeof(password), 0);
    trim_newline(password);

    // Open employee database using open system call
    int file_fd = open(EMPLOYEE_DB, O_RDONLY);
    if (file_fd < 0) {
        perror("Error opening employee database");
        send(client_socket, "Server error. Please try again later.\n", strlen("Server error. Please try again later.\n"), 0);
        return 0;
    }

    char file_username[50], file_password[50], emp_id[10];
    char line[BUFFER_SIZE];
    int found = 0;

    // Read the employee file line by line using system calls
    ssize_t bytes_read;
    int line_pos = 0;

    while ((bytes_read = read(file_fd, &line[line_pos], 1)) > 0) {
        if (line[line_pos] == '\n') {
            line[line_pos] = '\0';  // Null-terminate the line

            // Parse emp_id, username, and password from the line
            sscanf(line, "%s %s %s", emp_id, file_username, file_password);

            // Check if the credentials match
            if (strcmp(username, file_username) == 0 && strcmp(password, file_password) == 0) {
                found = 1;
                break;
            }

            line_pos = 0;  // Reset for the next line
        } else {
            line_pos++;  // Keep reading the current line
        }
    }

    close(file_fd);  // Close the file after reading

    if (found) {
        // Save the logged-in employee details
        strncpy(logged_in_emp_id, emp_id, sizeof(logged_in_emp_id) - 1);
        strncpy(logged_in_emp_username, file_username, sizeof(logged_in_emp_username) - 1);

        send(client_socket, "Employee login successful!\n", strlen("Employee login successful!\n"), 0);

        // Pause before showing the employee menu
        const char *pause_message = "Press Enter to continue...\n";
        send(client_socket, pause_message, strlen(pause_message), 0);

        memset(buffer, 0, BUFFER_SIZE);
        recv(client_socket, buffer, BUFFER_SIZE, 0);  // Wait for input (Enter)

        return 1;  // Successful login
    } else {
        send(client_socket, "Invalid employee credentials!\n", strlen("Invalid employee credentials!\n"), 0);
        return 0;  // Invalid credentials
    }
}


//Employee Menu
void employee_menu(int client_socket) {
    int choice;
    char buffer[BUFFER_SIZE];

    do {
        // Display employee menu
        snprintf(buffer, sizeof(buffer),
                "\n===== Employee Menu =====\n"
                "1. View Assigned Loan Applications\n"
                "2. Approve/ Reject Loans\n"
                "3. View Customer Transactions\n"
                "4. Change Password\n"
                "logout\n"
                "exit\n"
                "Enter your choice: ");
        send(client_socket, buffer, strlen(buffer), 0);

        memset(buffer, 0, BUFFER_SIZE);
        ssize_t bytes_received = recv(client_socket, buffer, sizeof(buffer), 0);
        if (bytes_received < 0) {
            perror("Error receiving choice");
            return; // Exit the menu on error
        }
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
            case 3: {
                char cust_id_buffer[BUFFER_SIZE];
                int target_cust_id;

                // Ask for the customer ID
                send(client_socket, "Enter the Customer ID to view transactions: ", 
                     strlen("Enter the Customer ID to view transactions: "), 0);
                memset(cust_id_buffer, 0, sizeof(cust_id_buffer));
                ssize_t bytes_cust_received = recv(client_socket, cust_id_buffer, sizeof(cust_id_buffer), 0);
                if (bytes_cust_received < 0) {
                    perror("Error receiving Customer ID");
                    return; // Exit the menu on error
                }
                target_cust_id = atoi(cust_id_buffer);

                if (target_cust_id <= 0) {
                    send(client_socket, "Invalid Customer ID.\n", strlen("Invalid Customer ID.\n"), 0);
                } else if (!customer_exists(target_cust_id)) {
                    send(client_socket, "Customer ID does not exist.\n", 
                         strlen("Customer ID does not exist.\n"), 0);
                } else {
                    // Display the customer's transaction history (passbook)
                    view_customer_transaction_history(client_socket, target_cust_id);
                }
                break;
            }
            case 4:
                change_employee_password(client_socket);
                break;
            case 5:
                // Logout
                send(client_socket, "Logged out.\n", strlen("Logged out.\n"), 0);
                break;
            case 6:
                // Exit the menu
                send(client_socket, "Exiting...\n", strlen("Exiting...\n"), 0);
                break;
            default:
                send(client_socket, "Invalid choice! Please try again.\n", 
                     strlen("Invalid choice! Please try again.\n"), 0);
                break;
        }
    } while (choice != 5 && choice != 6);
}


//Approve or Reject Loans
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


//View Employee's feedback
void view_employee_feedback(int client_socket, const char *feedback_file) {
    char line[BUFFER_SIZE];
    int file_fd;

    // Open feedback file
    file_fd = open(feedback_file, O_RDONLY);
    if (file_fd < 0) {
        perror("Error opening feedback file");
        send(client_socket, "No feedback available at the moment.\n", 
             strlen("No feedback available at the moment.\n"), 0);
        return;
    }

    // Send initial message
    send(client_socket, "Employee Feedbacks:\n", strlen("Employee Feedbacks:\n"), 0);

    // Read and send each line of feedback to the manager
    ssize_t bytes_read;
    while ((bytes_read = read(file_fd, line, sizeof(line) - 1)) > 0) {
        line[bytes_read] = '\0';  // Null-terminate the string
        send(client_socket, line, strlen(line), 0);
    }

    if (bytes_read < 0) {
        perror("Error reading feedback file");
        send(client_socket, "Error reading feedback file.\n", 
             strlen("Error reading feedback file.\n"), 0);
    }

    close(file_fd); // Close feedback file descriptor
}

int manager_login(int client_socket) {
    char username[50], password[50];
    char buffer[BUFFER_SIZE];
    memset(username, 0, sizeof(username));
    memset(password, 0, sizeof(password));
    memset(buffer, 0, BUFFER_SIZE);

    // Prompt for manager username
    send(client_socket, "Enter Manager Username: ", strlen("Enter Manager Username: "), 0);
    recv(client_socket, username, sizeof(username), 0);
    trim_newline(username);

    // Prompt for manager password
    send(client_socket, "Enter Manager Password: ", strlen("Enter Manager Password: "), 0);
    recv(client_socket, password, sizeof(password), 0);
    trim_newline(password);

    // Open manager database using open system call
    int file_fd = open(MANAGER_DB, O_RDONLY);
    if (file_fd < 0) {
        perror("Error opening manager database");
        send(client_socket, "Server error. Please try again later.\n", strlen("Server error. Please try again later.\n"), 0);
        return 0;
    }

    char file_username[50], file_password[50], mgr_id[10];
    char line[BUFFER_SIZE];
    int found = 0;

    // Read the manager file line by line using system calls
    ssize_t bytes_read;
    int line_pos = 0;

    while ((bytes_read = read(file_fd, &line[line_pos], 1)) > 0) {
        if (line[line_pos] == '\n') {
            line[line_pos] = '\0';  // Null-terminate the line

            // Parse mgr_id, username, and password from the line
            sscanf(line, "%s %s %s", mgr_id, file_username, file_password);

            // Check if the credentials match
            if (strcmp(username, file_username) == 0 && strcmp(password, file_password) == 0) {
                found = 1;
                break;
            }

            line_pos = 0;  // Reset for the next line
        } else {
            line_pos++;  // Keep reading the current line
        }
    }

    close(file_fd);  // Close the file after reading

    if (found) {
        send(client_socket, "Manager login successful!\n", strlen("Manager login successful!\n"), 0);

        // Pause before showing the manager menu
        const char *pause_message = "Press Enter to continue...\n";
        send(client_socket, pause_message, strlen(pause_message), 0);

        memset(buffer, 0, BUFFER_SIZE);
        recv(client_socket, buffer, BUFFER_SIZE, 0);  // Wait for input (Enter)

        return 1;  // Successful login
    } else {
        send(client_socket, "Invalid manager credentials!\n", strlen("Invalid manager credentials!\n"), 0);
        return 0;  // Invalid credentials
    }
}

// Function to implement manager menu
void manager_menu(int client_socket) {
    int choice, cust_id, loan_id;
    char buffer[BUFFER_SIZE];

    do {
        // Display manager menu
        const char *menu = 
                "\n===== Manager Menu =====\n"
                "1. Activate Customer Account\n"
                "2. Deactivate Customer Account\n"
                "3. Assign Loan Application to Employee\n"
                "4. View Employee Feedbacks\n"
                "Logout\n"
                "Exit\n"
                "Enter your choice: ";
        
        // Use write instead of send
        write(client_socket, menu, strlen(menu));
        
        // Clear buffer
        memset(buffer, 0, sizeof(buffer));
        
        // Use read instead of recv
        read(client_socket, buffer, sizeof(buffer));
        choice = atoi(buffer);

        switch (choice) {
            case 1:
                // Activate customer account
                write(client_socket, "Enter Customer ID to activate: ", 
                      strlen("Enter Customer ID to activate: "));
                read(client_socket, buffer, sizeof(buffer));
                cust_id = atoi(buffer);
                if (cust_id <= 0) {
                    write(client_socket, "Invalid Customer ID.\n", strlen("Invalid Customer ID.\n"));
                } else {
                    update_customer_status_func(client_socket, cust_id, 1); // 1 = active
                }
                break;

            case 2:
                // Deactivate customer account
                write(client_socket, "Enter Customer ID to deactivate: ", 
                      strlen("Enter Customer ID to deactivate: "));
                read(client_socket, buffer, sizeof(buffer));
                cust_id = atoi(buffer);
                if (cust_id <= 0) {
                    write(client_socket, "Invalid Customer ID.\n", strlen("Invalid Customer ID.\n"));
                } else {
                    update_customer_status_func(client_socket, cust_id, 0); // 0 = inactive
                }
                break;

            case 3:
                // Assign Loan Application to Employee
                int loan_id;
                char employee_id[BUFFER_SIZE];
                char formatted_emp_id[BUFFER_SIZE];
                #define EMP_ID_MAX_LEN 40  
                
                // Ask manager for loan ID
                write(client_socket, "Enter Loan ID: ", strlen("Enter Loan ID: "));
                read(client_socket, buffer, sizeof(buffer));
                loan_id = atoi(buffer); 

                // Ask manager for employee ID
                write(client_socket, "Enter Employee ID: ", strlen("Enter Employee ID: "));
                read(client_socket, employee_id, sizeof(employee_id));
                
                // Remove any trailing newline
                employee_id[strcspn(employee_id, "\n")] = 0;

                // Ensure the length of employee_id doesn't exceed the allowed maximum
                if (strlen(employee_id) > EMP_ID_MAX_LEN) {
                    write(client_socket, "Employee ID too long. Maximum allowed length is 40 characters.\n", 
                          strlen("Employee ID too long. Maximum allowed length is 40 characters.\n"));
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
                write(client_socket, "Logged out.\n", strlen("Logged out.\n"));
                break;

            default:
                write(client_socket, "Invalid choice! Please try again.\n", 
                      strlen("Invalid choice! Please try again.\n"));
                break;
        }
    } while (choice != 5);
}

// Update customer_status
int update_customer_status_func(int client_socket, int cust_id, int new_status) {
    int status_file;
    char line[BUFFER_SIZE];
    char all_statuses[BUFFER_SIZE * 100] = "";  // Buffer to hold all statuses
    int found = 0;

    // Open customer_status file
    status_file = open(CUSTOMER_STATUS_FILE, O_RDWR);
    if (status_file < 0) {
        perror("Error opening customer status file");
        write(client_socket, "Error opening customer status file.\n",
              strlen("Error opening customer status file.\n"));
        return 0;
    }

    // Read through the file and search for the customer
    while (read(status_file, line, sizeof(line)) > 0) {
        int temp_cust_id, temp_status;

        // Check for new line character to separate entries
        line[strcspn(line, "\n")] = 0;  // Remove the newline character for processing

        // Parse the line
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

    // Truncate the file and write all statuses back
    ftruncate(status_file, 0); // Truncate the file to zero length
    lseek(status_file, 0, SEEK_SET); // Move the file pointer to the beginning
    write(status_file, all_statuses, strlen(all_statuses));
    close(status_file); // Close the file

    // Notify manager of the status update
    if (new_status == 1) {
        write(client_socket, "Customer account activated.\n", strlen("Customer account activated.\n"));
    } else {
        write(client_socket, "Customer account deactivated.\n", strlen("Customer account deactivated.\n"));
    }

    return 1;
}

//Assign loan to employee
int assign_loan_to_employee_func(int client_socket, int loan_id, const char *employee_id) {
    char line[BUFFER_SIZE];
    char all_assignments[BUFFER_SIZE * 100] = "";  // Buffer to hold all assignments
    int found = 0;

    char formatted_loan_id[50];  // Buffer to store the formatted loan ID
    char formatted_line[BUFFER_SIZE];  // Buffer to store the formatted line for file output
    snprintf(formatted_loan_id, sizeof(formatted_loan_id), "loan_%d", loan_id);  // Format loan ID

    // Open loan_assignments file using system calls
    int assignment_file = open(LOAN_ASSIGNMENTS_FILE, O_RDWR);
    if (assignment_file < 0) {
        perror("Error opening loan assignment file");
        write(client_socket, "Error opening loan assignment file.\n", 
              strlen("Error opening loan assignment file.\n"));
        return 0;
    }

    // Read the file into buffer using system calls
    char file_buffer[BUFFER_SIZE * 10] = "";
    int bytes_read = read(assignment_file, file_buffer, sizeof(file_buffer) - 1);
    if (bytes_read < 0) {
        perror("Error reading loan assignment file");
        close(assignment_file);
        return 0;
    }
    file_buffer[bytes_read] = '\0';  // Ensure the buffer is null-terminated

    // Tokenize each line and process it
    char *line_ptr = strtok(file_buffer, "\n");
    while (line_ptr != NULL) {
        char temp_loan_id[50], temp_employee_id[50], temp_status[20];
        sscanf(line_ptr, "%s %s %s", temp_loan_id, temp_employee_id, temp_status);

        // If loan_id is found, update the employee_id but keep the status
        if (strcmp(temp_loan_id, formatted_loan_id) == 0) {
            found = 1;
            // Keep the existing status and update the employee ID
            snprintf(formatted_line, sizeof(formatted_line), "%s %s %s\n", formatted_loan_id, employee_id, temp_status);
        } else {
            // Preserve the original line
            snprintf(formatted_line, sizeof(formatted_line), "%s\n", line_ptr);
        }

        // Add the (possibly updated) line to all_assignments buffer
        strncat(all_assignments, formatted_line, sizeof(all_assignments) - strlen(all_assignments) - 1);

        // Get the next line
        line_ptr = strtok(NULL, "\n");
    }

    // If loan assignment not found, add a new entry with "pending" status
    if (!found) {
        snprintf(formatted_line, sizeof(formatted_line), "%s %s pending\n", formatted_loan_id, employee_id);
        strncat(all_assignments, formatted_line, sizeof(all_assignments) - strlen(all_assignments) - 1);
    }

    // Truncate the file and write all assignments back
    lseek(assignment_file, 0, SEEK_SET);  // Move file pointer to the beginning
    ftruncate(assignment_file, 0);  // Truncate the file to remove old content
    write(assignment_file, all_assignments, strlen(all_assignments));  // Write the updated content

    close(assignment_file);  // Close the file

    // Notify manager of the assignment update
    write(client_socket, "Loan assigned to employee successfully with 'pending' status.\n",
          strlen("Loan assigned to employee successfully with 'pending' status.\n"));

    return 1;
}

// Admin login function with hardcoded credentials
int admin_login(int client_socket) {
    char username[50], password[50];
    char buffer[BUFFER_SIZE];
    memset(username, 0, sizeof(username));
    memset(password, 0, sizeof(password));
    memset(buffer, 0, BUFFER_SIZE);  // Clear buffer

    // Prompt for username
    send(client_socket, "Enter Admin Username: ", strlen("Enter Admin Username: "), 0);
    recv(client_socket, username, sizeof(username), 0);
    trim_newline(username);  // Clean input

    // Prompt for password
    send(client_socket, "Enter Admin Password: ", strlen("Enter Admin Password: "), 0);
    recv(client_socket, password, sizeof(password), 0);
    trim_newline(password);  // Clean input

    // Check hardcoded credentials
    if (strcmp(username, "admin") == 0 && strcmp(password, "admin123") == 0) {
        send(client_socket, "Admin login successful!\n", strlen("Admin login successful!\n"), 0);

        // **Pause here before showing the admin menu**
        const char *pause_message = "Press Enter to continue...\n";
        send(client_socket, pause_message, strlen(pause_message), 0);

        // Wait for client to press Enter
        memset(buffer, 0, BUFFER_SIZE);
        recv(client_socket, buffer, BUFFER_SIZE, 0);  // Wait for input (Enter)

        return 1;  // Successful admin login
    } else {
        send(client_socket, "Invalid admin credentials!\n", strlen("Invalid admin credentials!\n"), 0);
        return 0;  // Invalid credentials
    }
}


// Function to get the next manager ID number
int get_next_manager_id_num() {
    FILE *file = fopen(MANAGER_DB, "r");
    if (!file) {
        return 1;  // If file doesn't exist, start with ID 1
    }

    char line[BUFFER_SIZE];
    int last_id = 0;
    int current_id;
    char manager_id_str[10];

    while (fgets(line, sizeof(line), file)) {
        if (sscanf(line, "%s", manager_id_str) == 1) {
            if (sscanf(manager_id_str, "mgr_%d", &current_id) == 1) {
                if (current_id > last_id) {
                    last_id = current_id;
                }
            }
        }
    }
    fclose(file);
    return last_id + 1;
}

// Function to add a new manager with a unique manager_id
int add_new_manager(int client_socket) {
    char username[50], password[50];
    char manager_id[10];
    int next_id;
    char buffer[BUFFER_SIZE];

    // Get the next manager ID
    next_id = get_next_manager_id_num();
    snprintf(manager_id, sizeof(manager_id), "mgr_%d", next_id);

    // Prompt for username
    send(client_socket, "Enter new Manager Username: ", strlen("Enter new Manager Username: "), 0);
    recv(client_socket, username, sizeof(username), 0);
    trim_newline(username);  // Clean input

    // Prompt for password
    send(client_socket, "Enter new Manager Password: ", strlen("Enter new Manager Password: "), 0);
    recv(client_socket, password, sizeof(password), 0);
    trim_newline(password);  // Clean input

    // Open manager_db.txt in append mode using system call
    int file_fd = open(MANAGER_DB, O_WRONLY | O_APPEND | O_CREAT, 0644);
    if (file_fd < 0) {
        perror("Error opening manager database");
        send(client_socket, "Error opening manager database.\n", strlen("Error opening manager database.\n"), 0);
        return 0;
    }

    // Prepare the new manager record: manager_id username password
    char new_manager_record[BUFFER_SIZE];
    snprintf(new_manager_record, sizeof(new_manager_record), "%s %s %s\n", manager_id, username, password);

    // Write the new manager record to the file
    ssize_t bytes_written = write(file_fd, new_manager_record, strlen(new_manager_record));
    if (bytes_written < 0) {
        perror("Error writing to manager database");
        close(file_fd);
        send(client_socket, "Error writing to manager database.\n", strlen("Error writing to manager database.\n"), 0);
        return 0;
    }

    close(file_fd);  // Close the file descriptor

    // Notify the user of successful addition
    snprintf(buffer, sizeof(buffer), "Manager added successfully! Manager ID: %s\n", manager_id);
    send(client_socket, buffer, strlen(buffer), 0);

    return 1;
}


// Function to implement admin menu
void admin_menu(int client_socket) {
    int choice;
    char buffer[BUFFER_SIZE];

    do {
        // Display admin menu
        snprintf(buffer, sizeof(buffer),
                "\n===== Admin Menu =====\n"
                "1. Add New Employee\n"
		"2. Add New Manager\n"
                "logout\n"
                "exit\n"
                "Enter your choice: ");
        write(client_socket, buffer, strlen(buffer)); // Send menu to client

        memset(buffer, 0, BUFFER_SIZE);
        read(client_socket, buffer, sizeof(buffer)); // Receive choice from client
        choice = atoi(buffer);

        switch (choice) {
            case 1:
                // Add New Employee
                add_new_employee(client_socket);
                break;
            case 2:
		//Add New Manager
		add_new_manager(client_socket);
		break;
            case 3:
                // Logout
                write(client_socket, "Logged out.\n", strlen("Logged out.\n"));
                break;

            case 4:
                // Exit (can be handled if needed)
                write(client_socket, "Exiting.\n", strlen("Exiting.\n"));
                break;

            default:
                write(client_socket, "Invalid choice! Please try again.\n",
                      strlen("Invalid choice! Please try again.\n"));
                break;
        }
    } while (choice != 3 && choice != 4); // Exit loop on logout or exit
}
