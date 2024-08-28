/* Name: Kanan Gupta
 * Roll No. MT2024073
 * Problem: Write a program, call fork and print the parent and child process id. 
 * */
 

#include <stdio.h>
#include <unistd.h>

int main() {
    pid_t pid;

    
    pid = fork();

    
    if (pid == 0) {
        printf("Child process with PID: %d\n", getpid());
	printf("Parent process. PPID: %d\n", getppid());
    } else {
        printf("Parent process. PID: %d\n",getpid());
	printf("Child PID: %d\n",pid);
    }

    return 0;
}


