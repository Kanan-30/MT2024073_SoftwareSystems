/* Name: Kanan Gupta
 * Roll No. MT2024073
 * Problem: Write a separate program using sigaction system call to catch the following signals.
b. SIGINT
 * */
 
#include<stdio.h>
#include<unistd.h>
#include<stdlib.h>
#include<signal.h>

	
void sigint_Handler(int sig) {
    printf("Caught SIGINT! \n");
    exit(0);
}

int main() {
    struct sigaction sa;
    sa.sa_handler = sigint_Handler;
    sa.sa_flags = 0;
    sigemptyset(&sa.sa_mask);

    sigaction(SIGINT, &sa, NULL);
	
    printf("Catching SIGINT Signal!\n");
    sleep(10); 
    printf("NO SIGINT within 10 seconds\n");
    exit(0);

}

