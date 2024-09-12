/* Name: Kanan Gupta
 * Roll No. MT2024073
 * Problem: Write a separate program using sigaction system call to catch the following signals.
a. SIGSEGV
 * */
 
#include<stdio.h>
#include<unistd.h>
#include<stdlib.h>
#include<signal.h>

	
void sigsev_Handler(int sig) {
    printf("Caught SIGSEGV! \n");
    exit(0);
}

int main() {
    struct sigaction sa;
    sa.sa_handler = sigsev_Handler;
    sa.sa_flags = 0;
    sigemptyset(&sa.sa_mask);

    sigaction(SIGSEGV, &sa, NULL);
	
    printf("Catching SIGSEV Signal!\n");
    sleep(10); 
    printf("No SIGSEV SIgnal within 10 seconds\n");
    exit(0);
}

