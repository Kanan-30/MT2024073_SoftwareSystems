/* Name: Kanan Gupta
 * Roll No. MT2024073
 * Problem: 
 * Write two programs: first program is waiting to catch SIGSTOP signal, the second program
will send the signal (using kill system call). Find out whether the first program is able to catch
the signal or not.
 * */
 
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>

int main(int argc, char *argv[]) {
    if (argc != 2) {
        printf("PLease enter pid of the process: %s \n", argv[0]);
        return 1;
    }

    pid_t target_pid = atoi(argv[1]);
  
       
    if (kill(target_pid, SIGSTOP) == 0) {
        printf("Signal SIGSTOP sent to process %d\n", target_pid);
    } else {
        perror("Failed to send signal");
    }

    return 0;
}


