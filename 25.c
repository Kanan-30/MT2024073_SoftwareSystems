/* Name: Kanan Gupta
 * Roll No. MT2024073
 * Problem: Write a program to create three child processes. The parent should wait for a particular child (use
waitpid system call).
 * */
 

#include <stdio.h>
#include <unistd.h>
#include <sys/wait.h>

int main() {
    pid_t pid1, pid2, pid3;

    pid1 = fork();
    if (pid1 == 0) {
        printf("Child 1 (PID: %d)\n", getpid());
        sleep(2);
        return 0;
    }

    pid2 = fork();
    if (pid2 == 0) {
        printf("Child 2 (PID: %d)\n", getpid());
        sleep(4);
        return 0;
    }

    pid3 = fork();
    if (pid3 == 0) {
        printf("Child 3 (PID: %d)\n", getpid());
        sleep(6);
        return 0;
    }

    printf("Parent waiting for Child 2 (PID: %d)\n", pid2);
    waitpid(pid2, NULL, 0);
    printf("Child 2 (PID: %d) done\n", pid2);

    return 0;
}


