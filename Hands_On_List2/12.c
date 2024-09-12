/* Name: Kanan Gupta
 * Roll No. MT2024073
 * Problem: Write a program to create an orphan process. Use kill system call to send SIGKILL signal to
the parent process from the child process.
 * */
 

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <sys/wait.h>

int main() {
    pid_t pid = fork();

    if (pid == 0) {
        printf("Child process executing: PID: %d\n", getpid());
        printf("Child sent SIGKILL to parent process with PID: %d\n", getppid());

        kill(getppid(), SIGKILL);

        printf("Parent process killed, chid became orphan\n");

        sleep(3);

        printf("Child process exiting, new Parent: (PID: %d)\n", getppid());
	exit(0);

    } else {
        printf("Parent process PID: %d\n", getpid());
        printf("Parent process waiting for child to execute...\n");

        sleep(5);
    }
wait(0);
    return 0;
}

