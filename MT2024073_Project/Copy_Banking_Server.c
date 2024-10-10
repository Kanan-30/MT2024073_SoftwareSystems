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

#define PORT 5080
#define MAX_CLIENTS 10
#define BUFFER_SIZE 1048
#define USERNAME_SIZE 50
#define PASSWORD_SIZE 50
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
    int fd = open(CUSTOMER_DB, O_RDONLY);
    if (fd == -1) {
        // File doesn't exist, start with ID 1
        return 1;
    }

    char buffer[BUFFER_SIZE];
    char line[BUFFER_SIZE];
    ssize_t bytes_read;
    int last_id = 0;
    int cust_id = 0;
    size_t line_len = 0;

    // Read file contents in chunks
    while ((bytes_read = read(fd, buffer, sizeof(buffer))) > 0) {
        for (ssize_t i = 0; i < bytes_read; i++) {
            if (buffer[i] == '\n' || buffer[i] == '\0') {
                // Process the line when a newline or null character is found
                line[line_len] = '\0';  // Null-terminate the line

                if (sscanf(line, "%d", &cust_id) == 1) {
                    if (cust_id > last_id) {
                        last_id = cust_id;
                    }
                }

                // Reset line length for the next line
                line_len = 0;
            } else {
                // Build up the line
                if (line_len < sizeof(line) - 1) {
                    line[line_len++] = buffer[i];
                }
            }
        }
    }

    // Handle read errors
    if (bytes_read == -1) {
        perror("Error reading file");
        close(fd);
        return -1;
    }

    // Close the file descriptor
    close(fd);

    return last_id + 1;
}
// Function to check if a customer exists by cust_id
int customer_exists(int cust_id) {
    int fd = open(CUSTOMER_DB, O_RDONLY);
    if (fd == -1) {
        // File couldn't be opened
        return 0;
    }

    char buffer[BUFFER_SIZE];
    char line[BUFFER_SIZE];
    ssize_t bytes_read;
    int current_id = 0;
    size_t line_len = 0;

    // Read file contents in chunks
    while ((bytes_read = read(fd, buffer, sizeof(buffer))) > 0) {
        for (ssize_t i = 0; i < bytes_read; i++) {
            if (buffer[i] == '\n' || buffer[i] == '\0') {
                // Process the line when a newline or null character is found
                line[line_len] = '\0';  // Null-terminate the line

                if (sscanf(line, "%d", &current_id) == 1) {
                    if (current_id == cust_id) {
                        close(fd);  // Close file before returning
                        return 1;
                    }
                }

                // Reset line length for the next line
                line_len = 0;
            } else {
                // Build up the line
                if (line_len < sizeof(line) - 1) {
                    line[line_len++] = buffer[i];
                }
            }
        }
    }

    // Handle read errors
    if (bytes_read == -1) {
        perror("Error reading file");
        close(fd);
        return 0;
    }

    // Close the file descriptor
    close(fd);

    return 0;
}

// Function to get customer's current amount
double get_customer_amount(int cust_id) {
    int fd = open(CUSTOMER_DB, O_RDONLY);
    if (fd == -1) {
        perror("Error opening customer database for reading amount");
        return -1.0;
    }

    char buffer[BUFFER_SIZE];
    char line[BUFFER_SIZE];
    ssize_t bytes_read;
    double amount = -1.0;
    int current_id = 0;
    char username[50], password[50];
    double current_amount;
    size_t line_len = 0;

    // Read file contents in chunks
    while ((bytes_read = read(fd, buffer, sizeof(buffer))) > 0) {
        for (ssize_t i = 0; i < bytes_read; i++) {
            if (buffer[i] == '\n' || buffer[i] == '\0') {
                // Process the line when a newline or null character is found
                line[line_len] = '\0';  // Null-terminate the line

                // Parse the line to extract customer ID and amount
                int parsed = sscanf(line, "%d %s %s %lf", &current_id, username, password, &current_amount);
                if (parsed == 4 && current_id == cust_id) {
                    amount = current_amount;
                    close(fd);  // Close file before returning
                    return amount;
                }

                // Reset line length for the next line
                line_len = 0;
            } else {
                // Build up the line
                if (line_len < sizeof(line) - 1) {
                    line[line_len++] = buffer[i];
                }
            }
        }
    }

    // Handle read errors
    if (bytes_read == -1) {
        perror("Error reading file");
        close(fd);
        return -1.0;
    }

    // Close the file descriptor
    close(fd);

    return amount;  // Return the found amount or -1.0 if not found
}

// Function to update customer's amount in the database
/*int update_customer_amount(int cust_id, double new_amount) {
    int fd = open(CUSTOMER_DB, O_RDWR);  // Open file for reading and writing
    if (fd == -1) {
        perror("Error opening customer database for updating");
        return 0;
    }

    // Lock the file for writing
    if (flock(fd, LOCK_EX) < 0) {
        perror("Error locking the file");
        close(fd);
        return 0;
    }

    char buffer[BUFFER_SIZE];
    char line[BUFFER_SIZE];
    ssize_t bytes_read;
    off_t pos = 0;  // Position in file
    int updated = 0;
    int current_id;
    char username[50], password[50];
    double current_amount;
    size_t line_len = 0;

    // Read file contents in chunks
    while ((bytes_read = read(fd, buffer, sizeof(buffer))) > 0) {
        for (ssize_t i = 0; i < bytes_read; i++) {
            if (buffer[i] == '\n' || buffer[i] == '\0') {
                // Process the line when a newline or null character is found
                line[line_len] = '\0';  // Null-terminate the line

                // Parse the line to extract customer ID and amount
                int parsed = sscanf(line, "%d %s %s %lf", &current_id, username, password, &current_amount);
                if (parsed == 4 && current_id == cust_id) {
                    // Find the position of the third space in the line
                    int space_count = 0;
                    int j;
                    for (j = 0; line[j] != '\0'; j++) {
                        if (line[j] == ' ') {
                            space_count++;
                            if (space_count == 3) {
                                break;
                            }
                        }
                    }

                    if (space_count < 3) {
                        flock(fd, LOCK_UN);
                        close(fd);
                        return 0;  // Invalid format
                    }

                    // Prepare the new amount string
                    char new_amount_str[20];
                    snprintf(new_amount_str, sizeof(new_amount_str), "%.2lf", new_amount);

                    // Move file pointer to the position just after the third space
                    off_t offset = pos + j + 1;  // +1 to skip the space before the amount
                    lseek(fd, offset, SEEK_SET);

                    // Overwrite the amount
                    write(fd, new_amount_str, strlen(new_amount_str));

                    // Fill remaining spaces with blank spaces if necessary to avoid truncation
                    ssize_t remaining = strlen(line) - (j + 1 + strlen(new_amount_str));
                    for (ssize_t k = 0; k < remaining; k++) {
                        write(fd, " ", 1);  // Ensure proper format and avoid truncation
                    }

                    updated = 1;
                    break;
                }

                // Reset line length for the next line
                line_len = 0;
                pos += i + 1;  // Update the position after processing a line
            } else {
                // Build up the line
                if (line_len < sizeof(line) - 1) {
                    line[line_len++] = buffer[i];
                }
            }
        }
    }

    // Handle read errors
    if (bytes_read == -1) {
        perror("Error reading file");
    }

    // Unlock and close the file
    flock(fd, LOCK_UN);
    close(fd);

    return updated;
}*/
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
    int fd = open(LOAN_DB, O_RDONLY);  // Open file in read-only mode
    if (fd == -1) {
        // If file doesn't exist or can't be opened, start with loan_1
        return 1;
    }

    char buffer[BUFFER_SIZE];
    char line[BUFFER_SIZE];
    ssize_t bytes_read;
    int last_id = 0;
    int loan_id_num = 0;
    size_t line_len = 0;

    // Read file contents in chunks
    while ((bytes_read = read(fd, buffer, sizeof(buffer))) > 0) {
        for (ssize_t i = 0; i < bytes_read; i++) {
            if (buffer[i] == '\n' || buffer[i] == '\0') {
                // Process the line when a newline or null character is found
                line[line_len] = '\0';  // Null-terminate the line

                // Parse the line to extract loan ID (loan_x)
                if (sscanf(line, "loan_%d", &loan_id_num) == 1) {
                    if (loan_id_num > last_id) {
                        last_id = loan_id_num;
                    }
                }

                // Reset line length for the next line
                line_len = 0;
            } else {
                // Build up the line
                if (line_len < sizeof(line) - 1) {
                    line[line_len++] = buffer[i];
                }
            }
        }
    }

    // Handle read errors
    if (bytes_read == -1) {
        perror("Error reading file");
    }

    close(fd);  // Close the file

    return last_id + 1;  // Return the next loan ID
}
// Login for customer, employee, manager and admin
int login_user(int client_socket, const char *role_file, const char *role) {
    char buffer[BUFFER_SIZE], username[USERNAME_SIZE], password[PASSWORD_SIZE];
    char file_emp_id[10], file_username[USERNAME_SIZE], file_password[PASSWORD_SIZE];
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

    int found = 0;
    int current_cust_id = -1;
    double current_amount = 0.00;
    size_t line_len = 0;
    ssize_t bytes_read;
    
    // Read the file contents using read() system call
    while ((bytes_read = read(file_fd, buffer, sizeof(buffer))) > 0) {
        for (ssize_t i = 0; i < bytes_read; i++) {
            if (buffer[i] == '\n' || buffer[i] == '\0') {
                // Process the line when a newline or null character is found
                line[line_len] = '\0';  // Null-terminate the line

                if (strcmp(role, "Customer") == 0) {
                    // For customers: cust_id username password amount
                    int parsed = sscanf(line, "%d %s %s %lf", &current_cust_id, file_username, file_password, &current_amount);
                    if (parsed < 4) continue;  // Invalid line format
                } else if (strcmp(role, "Employee") == 0) {
                    // For employees: emp_id username password
                    int parsed = sscanf(line, "%s %s %s", file_emp_id, file_username, file_password);
                    if (parsed < 3) continue;  // Invalid line format
                } else {
                    // For other roles: username password
                    int parsed = sscanf(line, "%s %s", file_username, file_password);
                    if (parsed < 2) continue;  // Invalid line format
                }

                if (strcmp(file_username, username) == 0) {
                    found = 1;  // Username found
                    break;
                }

                // Reset line length for the next line
                line_len = 0;
            } else {
                // Build up the line
                if (line_len < sizeof(line) - 1) {
                    line[line_len++] = buffer[i];
                }
            }
        }
    }

    // Handle read errors
    if (bytes_read == -1) {
        perror("Error reading file");
    }

    close(file_fd);  // Close the file descriptor

    if (found) {
        // Ask for password if username exists
        memset(password, 0, sizeof(password));
        send(client_socket, "Enter Password: ", strlen("Enter Password: "), 0);
        recv(client_socket, password, sizeof(password), 0);
        trim_newline(password);  // Clean input

        if (strcmp(file_password, password) == 0) {
            send(client_socket, "Login successful!\n", strlen("Login successful!\n"), 0);
            strcpy(logged_in_username, username);
            if (strcmp(role, "Customer") == 0) {
                logged_in_cust_id = current_cust_id;
                logged_in_amount = current_amount;
            }
            if (strcmp(role, "Employee") == 0) {
                strcpy(logged_in_emp_id, file_emp_id);
            }
            return 1;  // Successful login
        } else {
            send(client_socket, "Login failed! Incorrect password.\n",
                 strlen("Login failed! Incorrect password.\n"), 0);
            return 0;  // Password mismatch
        }
    } else {
        // If username not found, prompt to create a new account
        send(client_socket, "Username not found! Create a new account? (yes/no): ",
             strlen("Username not found! Create a new account? (yes/no): "), 0);
        recv(client_socket, buffer, sizeof(buffer), 0);
        trim_newline(buffer);  // Clean input

        if (strcasecmp(buffer, "yes") == 0) {
            return create_new_account(client_socket, role_file, role);  // Create account if yes
        } else {
            send(client_socket, "Login cancelled.\n", strlen("Login cancelled.\n"), 0);
            return 0;  // Cancel if no
        }
    }
}


// Function to create a new account
int create_new_account(int client_socket, const char *role_file, const char *role) {
    char username[50], password[50], buffer[BUFFER_SIZE];
    int file_fd;

    if (strcmp(role, "Customer") == 0) {
        // For customers, open file in append mode with system call
        file_fd = open(role_file, O_WRONLY | O_APPEND | O_CREAT, 0644);
        if (file_fd < 0) {
            perror("Error opening customer file");
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

        // Create the customer record string: "cust_id username password amount\n"
        snprintf(buffer, sizeof(buffer), "%d %s %s %.2lf\n", cust_id, username, password, initial_amount);

        // Write the new customer record using write() system call
        if (write(file_fd, buffer, strlen(buffer)) < 0) {
            perror("Error writing to file");
            close(file_fd);
            return 0;
        }

        close(file_fd); // Close the file descriptor

        // Send success message
        char success_msg[BUFFER_SIZE];
        snprintf(success_msg, sizeof(success_msg), "Account created successfully! Your Customer ID is %d.\n", cust_id);
        send(client_socket, success_msg, strlen(success_msg), 0);

        return 1;
    } else {
        // For other roles, similar approach
        file_fd = open(role_file, O_WRONLY | O_APPEND | O_CREAT, 0644);
        if (file_fd < 0) {
            perror("Error opening file");
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

        // Create the user record string: "username password\n"
        snprintf(buffer, sizeof(buffer), "%s %s\n", username, password);

        // Write the new user record using write() system call
        if (write(file_fd, buffer, strlen(buffer)) < 0) {
            perror("Error writing to file");
            close(file_fd);
            return 0;
        }

        close(file_fd); // Close the file descriptor

        // Send success message
        send(client_socket, "Account created successfully!\n", strlen("Account created successfully!\n"), 0);
        return 1;
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

//View Transaction History
ssize_t read_line(int fd, char *buffer, size_t size) {
    ssize_t bytes_read;
    size_t total_bytes = 0;
    char c;

    while ((bytes_read = read(fd, &c, 1)) > 0 && total_bytes < size - 1) {
        if (c == '\n') {
            buffer[total_bytes] = '\0';  // Null-terminate the string
            return total_bytes;
        }
        buffer[total_bytes++] = c;
    }

    buffer[total_bytes] = '\0';  // Null-terminate at end of file or buffer
    return (bytes_read == 0 && total_bytes == 0) ? -1 : total_bytes;
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

//Apply for loan
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

    // Read the customer database line by line
    while (read_line(file, line, sizeof(line)) > 0) {
        int temp_cust_id;
        sscanf(line, "%d %s %s %f", &temp_cust_id, file_username, file_password, &balance);
        if (temp_cust_id == cust_id) {
            found = 1;
            break;
        }
    }

    close(file);  // Close customer database file descriptor

    if (!found) {
        send(client_socket, "Customer ID not found!\n", strlen("Customer ID not found!\n"), 0);
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
    int loan_db_file = open(LOAN_DB, O_WRONLY | O_APPEND | O_CREAT, 0644);
    if (loan_db_file < 0) {
        perror("Error opening loan database");
        return 0;
    }

    // Construct the loan record (loan_id, cust_id, loan_amount)
    snprintf(buffer, sizeof(buffer), "loan_%d %d %.2f\n", loan_id, cust_id, loan_amt);

    // Write the loan record to the loan_db file
    if (write(loan_db_file, buffer, strlen(buffer)) < 0) {
        perror("Error writing to loan database");
        close(loan_db_file);
        return 0;
    }

    close(loan_db_file);  // Close loan database file descriptor

    // Notify the customer that the loan was applied successfully
    send(client_socket, "Loan applied successfully!\n", strlen("Loan applied successfully!\n"), 0);

    return 1;
}

//Check loan status
int check_loan_status(int client_socket, int customer_id) {
    int loan_db_file = open(LOAN_DB, O_RDONLY);
    if (loan_db_file < 0) {
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
    ssize_t bytes_read;
    while ((bytes_read = read_line(loan_db_file, line, sizeof(line))) > 0) {
        int loan_id, db_cust_id;
        double amount;

        // Parse loan_id, customer_id, and amount
        if (sscanf(line, "loan_%d %d %lf", &loan_id, &db_cust_id, &amount) == 3) {
            if (db_cust_id == customer_id) {
                loan_found = 1;

                // Check loan status in loan_assignments.txt
                int assignment_file = open(LOAN_ASSIGNMENTS_FILE, O_RDONLY);
                if (assignment_file < 0) {
                    perror("Error opening loan assignment file");
                    send(client_socket, "Error opening loan assignment file.\n", 
                         strlen("Error opening loan assignment file.\n"), 0);
                    close(loan_db_file);
                    return 0;
                }

                char assignment_line[BUFFER_SIZE];
                char status[20] = "Not Assigned";  // Default status if not found
                while (read_line(assignment_file, assignment_line, sizeof(assignment_line)) > 0) {
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
                close(assignment_file);  // Close loan assignments file descriptor

                // Append loan information and status to the response
                char loan_info[BUFFER_SIZE];
                snprintf(loan_info, sizeof(loan_info), "loan_%d\t%.2lf\t%s\n", loan_id, amount, status);
                strncat(response, loan_info, sizeof(response) - strlen(response) - 1);
            }
        }
    }

    close(loan_db_file);  // Close loan database file descriptor

    if (!loan_found) {
        send(client_socket, "No loans found for your account.\n", 
             strlen("No loans found for your account.\n"), 0);
    } else {
        // Send the response to the customer
        send(client_socket, response, strlen(response), 0);
    }

    return 1;
}

//Change Customer Password
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

    // Open the customer database file for reading
    int file = open(CUSTOMER_DB, O_RDONLY);
    if (file < 0) {
        perror("Error opening customer database");
        send(client_socket, "Error opening customer database.\n", strlen("Error opening customer database.\n"), 0);
        return 0;
    }

    // Create a temporary file for writing
    int temp = open("customer_db.tmp", O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if (temp < 0) {
        perror("Error opening temporary file");
        close(file);
        send(client_socket, "Error opening temporary file.\n", strlen("Error opening temporary file.\n"), 0);
        return 0;
    }

    char line[BUFFER_SIZE];
    int updated = 0;

    // Loop through the file to find the matching username
    while (read_line(file, line, sizeof(line)) > 0) {
        char username[50], password[50];
        int cust_id;
        double amount;

        sscanf(line, "%d %s %s %lf", &cust_id, username, password, &amount);

        if (strcmp(username, logged_in_username) == 0) {
            // Update the password for the logged-in user
            dprintf(temp, "%d %s %s %.2lf\n", cust_id, username, new_password, amount);
            updated = 1;
        } else {
            // Write the line as-is
            write(temp, line, strlen(line));
        }
    }

    close(file);
    close(temp);

    // If the password was updated, rename the temp file to replace the original file
    if (updated) {
        if (rename("customer_db.tmp", CUSTOMER_DB) < 0) {
            perror("Error renaming temporary file");
            send(client_socket, "Error updating password.\n", strlen("Error updating password.\n"), 0);
            return 0;
        }
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

    // Step 1: Open employee_db to display list of employees
    file_fd = open(employee_db, O_RDONLY);
    if (file_fd < 0) {
        perror("Error opening employee database");
        send(client_socket, "Server error. Please try again later.\n", strlen("Server error. Please try again later.\n"), 0);
        return;
    }

    // Step 2: Read and display the list of employees
    send(client_socket, "List of employees available for feedback:\n", strlen("List of employees available for feedback:\n"), 0);

    while (read_line(file_fd, line, sizeof(line)) > 0) {
        // Assuming employee lines are in the format: emp_id username password
        int parsed = sscanf(line, "%s %s", emp_id, emp_username);
        if (parsed < 2) continue; // Invalid line format, skip

        // Send employee info to the customer
        snprintf(buffer, sizeof(buffer), "Employee ID: %s, Username: %s\n", emp_id, emp_username);
        send(client_socket, buffer, strlen(buffer), 0);
    }

    close(file_fd); // Close employee file

    // Step 3: Ask the customer to select an employee for feedback
    send(client_socket, "Enter the Employee ID you want to give feedback to: ", strlen("Enter the Employee ID you want to give feedback to: "), 0);
    recv(client_socket, selected_emp_id, sizeof(selected_emp_id), 0);
    trim_newline_and_control_chars(selected_emp_id); // Clean input

    // Step 4: Ask the customer to provide feedback
    send(client_socket, "Enter your feedback: ", strlen("Enter your feedback: "), 0);
    recv(client_socket, feedback, sizeof(feedback), 0);
    trim_newline_and_control_chars(feedback); // Clean input

    // Step 5: Store the feedback in the feedback file
    file_fd = open(feedback_file, O_WRONLY | O_APPEND | O_CREAT, 0644);
    if (file_fd < 0) {
        perror("Error opening feedback file");
        send(client_socket, "Server error. Please try again later.\n", strlen("Server error. Please try again later.\n"), 0);
        return;
    }

    // Write feedback to the file (emp_id: feedback)
    snprintf(buffer, sizeof(buffer), "Employee ID: %s, Feedback: %s\n", selected_emp_id, feedback);
    write(file_fd, buffer, strlen(buffer));

    close(file_fd); // Close feedback file

    send(client_socket, "Feedback submitted successfully!\n", strlen("Feedback submitted successfully!\n"), 0);
}

//Log Transactions

void log_transaction(int cust_id, const char *transaction_type, double amount) {
    // Open transaction log file in append mode
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
    char log_entry[BUFFER_SIZE];  // Make sure to define BUFFER_SIZE
    snprintf(log_entry, sizeof(log_entry), "%d, %s, %.2lf, %s\n", cust_id, transaction_type, amount, timestamp);

    // Write transaction log entry to file
    write(file_fd, log_entry, strlen(log_entry));

    // Close the file descriptor
    close(file_fd);
}

// Function to implement the customer menu
void customer_menu(int client_socket) {
    int choice;
    char buffer[BUFFER_SIZE];

    do {
        // Display menu options to the customer
        snprintf(buffer, sizeof(buffer),
                "\nCustomer Menu:\n"
                "1. Change Password\n"
                "2. View Account Balance\n"
                "3. Deposit Money\n"
                "4. Withdraw Money\n"
                "5. Transfer Funds\n"
                "6. Apply for a Loan\n"
                "7. Check your loan status\n"
                "8. View Transaction History\n"
                "9. Provide Feedback\n"
                "10. Logout\n"
                "Enter your choice: ");
        send(client_socket, buffer, strlen(buffer), 0);

        // Clear the buffer and receive user's choice
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
                    int customer_id;
                    send(client_socket, "Please enter your customer ID: ", strlen("Please enter your customer ID: "), 0);
                    recv(client_socket, buffer, sizeof(buffer), 0);
                    customer_id = atoi(buffer);
                    check_loan_status(client_socket, customer_id);
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

// Function to get the next employee ID in the format emp_1, emp_2, etc.
int get_next_employee_id_num() {
    int file_fd = open(EMPLOYEE_DB, O_RDONLY);
    if (file_fd < 0) {
        // If the file doesn't exist, start with ID 1
        return 1;
    }

    char line[BUFFER_SIZE];
    int last_id = 0;
    int current_id;
    char emp_id_str[10];

    // Read the file content using read() system call
    ssize_t bytes_read;
    while ((bytes_read = read(file_fd, line, sizeof(line) - 1)) > 0) {
        line[bytes_read] = '\0'; // Null-terminate the buffer
        char *line_ptr = line;

        // Process each line
        while (*line_ptr) {
            if (sscanf(line_ptr, "%s", emp_id_str) == 1) {
                if (sscanf(emp_id_str, "emp_%d", &current_id) == 1) {
                    if (current_id > last_id) {
                        last_id = current_id;
                    }
                }
            }
            // Move to the next line
            line_ptr += strlen(line_ptr) + 1; // Advance pointer by line length + 1 for newline
        }
    }

    close(file_fd); // Close the file descriptor
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

    // Open loan assignments file using system calls
    int assignment_fd = open(LOAN_ASSIGNMENTS_FILE, O_RDONLY);
    if (assignment_fd < 0) {
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

    // Read from the loan assignments file
    ssize_t bytes_read;
    while ((bytes_read = read(assignment_fd, line, sizeof(line) - 1)) > 0) {
        line[bytes_read] = '\0'; // Null-terminate the string
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
    close(assignment_fd); // Close the loan assignments file descriptor

    if (count == 0) {
        send(client_socket, "No loan applications assigned to you.\n", strlen("No loan applications assigned to you.\n"), 0);
        return 1;
    }

    // Open loans_db.txt to fetch loan details using system calls
    int loan_db_fd = open(LOAN_DB, O_RDONLY);
    if (loan_db_fd < 0) {
        perror("Error opening loans database file");
        send(client_socket, "Error opening loans database file.\n", strlen("Error opening loans database file.\n"), 0);
        return 0;
    }

    // Prepare the response
    char response[BUFFER_SIZE * 10] = "Your Assigned Loan Applications:\nLoan ID\tCustomer ID\tAmount\tStatus\n";
    for (int i = 0; i < count; i++) {
        lseek(loan_db_fd, 0, SEEK_SET);  // Reset file pointer to beginning
        int found = 0;
        char loan_line[BUFFER_SIZE];
        int cust_id;
        double amount;

        // Format loan_id as "loan_x"
        char loan_id_str[20];
        snprintf(loan_id_str, sizeof(loan_id_str), "loan_%d", loan_ids[i]);

        while ((bytes_read = read(loan_db_fd, loan_line, sizeof(loan_line) - 1)) > 0) {
            loan_line[bytes_read] = '\0'; // Null-terminate the string
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

    close(loan_db_fd); // Close the loans database file descriptor

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
    // Open the transaction log file in read mode
    int file_fd = open("transactions_db.txt", O_RDONLY);
    if (file_fd < 0) {
        perror("Error opening transaction log file");
        send(client_socket, "Error retrieving transaction history.\n", strlen("Error retrieving transaction history.\n"), 0);
        return;
    }

    char line[BUFFER_SIZE];
    char response[BUFFER_SIZE * 10] = "Customer Transaction History (Passbook):\n";
    int has_transactions = 0;

    // Read lines from the transaction log file
    ssize_t bytes_read;
    while ((bytes_read = read(file_fd, line, sizeof(line) - 1)) > 0) {
        line[bytes_read] = '\0'; // Null-terminate the line
        int current_id;
        char transaction_type[50];
        double amount;
        char timestamp[50];

        // Extracting transaction details from the log
        if (sscanf(line, "%d, %49[^,], %lf, %49[^\n]", &current_id, transaction_type, &amount, timestamp) == 4) {
            // Check if the transaction belongs to the customer
            if (current_id == cust_id) {
                has_transactions = 1;
                char transaction_details[BUFFER_SIZE];
                snprintf(transaction_details, sizeof(transaction_details), "%s: %.2lf on %s\n", transaction_type, amount, timestamp);
                strncat(response, transaction_details, sizeof(response) - strlen(response) - 1);
            }
        }
    }

    // Close the transaction log file
    close(file_fd);

    if (!has_transactions) {
        strncat(response, "No transactions found for this customer.\n", sizeof(response) - strlen(response) - 1);
    }

    // Send the transaction history to the employee
    send(client_socket, response, strlen(response), 0);
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
                "5. Logout\n"
                "6. Exit\n"
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
    int file = open(LOAN_ASSIGNMENTS_FILE, O_RDWR);
    char buffer[BUFFER_SIZE];
    char loan_id[50], status[20];
    int choice;

    if (file < 0) {
        send(client_socket, "Error opening loan assignments file.\n", 
             strlen("Error opening loan assignments file.\n"), 0);
        return;
    }

    // Show the assigned loans first
    view_assigned_loans(client_socket);

    // Ask employee to enter Loan ID
    send(client_socket, "Enter Loan ID to approve/reject: ", 
         strlen("Enter Loan ID to approve/reject: "), 0);
    recv(client_socket, loan_id, sizeof(loan_id), 0);
    strtok(loan_id, "\n");

    // Ask for approval or rejection
    send(client_socket, "Enter 1 to Approve, 2 to Reject: ", 
         strlen("Enter 1 to Approve, 2 to Reject: "), 0);
    recv(client_socket, buffer, sizeof(buffer), 0);
    choice = atoi(buffer);

    if (choice == 1) {
        strcpy(status, "Approved");
    } else if (choice == 2) {
        strcpy(status, "Rejected");
    } else {
        send(client_socket, "Invalid choice!\n", strlen("Invalid choice!\n"), 0);
        close(file);
        return;
    }

    // Prepare to read and update the file
    char line[BUFFER_SIZE];
    char updated_loans[BUFFER_SIZE * 100] = "";
    int loan_found = 0;
    ssize_t bytes_read;

    while ((bytes_read = read(file, line, sizeof(line))) > 0) {
        line[bytes_read] = '\0';  // Null-terminate the line
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

    // Truncate the original file and write back the updated loans
    lseek(file, 0, SEEK_SET);  // Reset file pointer to the beginning
    ftruncate(file, 0);        // Truncate the file to zero length
    write(file, updated_loans, strlen(updated_loans));  // Write updated loans

    close(file);

    if (loan_found) {
        send(client_socket, "Loan status updated successfully.\n", 
             strlen("Loan status updated successfully.\n"), 0);
    } else {
        send(client_socket, "Loan ID not found.\n", 
             strlen("Loan ID not found.\n"), 0);
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
    int assignment_file;
    char line[BUFFER_SIZE];
    char all_assignments[BUFFER_SIZE * 100] = "";  // Buffer to hold all assignments
    int found = 0;

    char formatted_loan_id[50];  // Buffer to store the formatted loan ID
    char formatted_line[BUFFER_SIZE];  // Buffer to store the formatted line for file output
    snprintf(formatted_loan_id, sizeof(formatted_loan_id), "loan_%d", loan_id);  // Format loan ID

    // Open loan_assignments file
    assignment_file = open(LOAN_ASSIGNMENTS_FILE, O_RDWR);
    if (assignment_file < 0) {
        perror("Error opening loan assignment file");
        write(client_socket, "Error opening loan assignment file.\n", strlen("Error opening loan assignment file.\n"));
        return 0;
    }

    // Read through the file and check for existing assignment for the loan
    while (read(assignment_file, line, sizeof(line)) > 0) {
        // Check for new line character to separate entries
        line[strcspn(line, "\n")] = 0;  // Remove the newline character for processing

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

    // Truncate the file and write all assignments back
    ftruncate(assignment_file, 0); // Truncate the file to zero length
    lseek(assignment_file, 0, SEEK_SET); // Move the file pointer to the beginning
    write(assignment_file, all_assignments, strlen(all_assignments));
    close(assignment_file); // Close the file

    // Notify manager of the assignment update
    write(client_socket, "Loan assigned to employee successfully with 'pending' status.\n",
          strlen("Loan assigned to employee successfully with 'pending' status.\n"));

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
                "2. Logout\n"
                "3. Exit\n"
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
                // Logout
                write(client_socket, "Logged out.\n", strlen("Logged out.\n"));
                break;

            case 3:
                // Exit (can be handled if needed)
                write(client_socket, "Exiting.\n", strlen("Exiting.\n"));
                break;

            default:
                write(client_socket, "Invalid choice! Please try again.\n",
                      strlen("Invalid choice! Please try again.\n"));
                break;
        }
    } while (choice != 2 && choice != 3); // Exit loop on logout or exit
}
