/* Name: Kanan Gupta
 * Roll No. MT2024073
 * Problem: Write a separate program using sigaction system call to catch the following signals.
c. SIGFPE
 * */
 
#include<stdio.h>
#include<unistd.h>
#include<stdlib.h>
#include<signal.h>

	
void sigfpe_Handler(int sig) {
    printf("Caught SIGFPE! \n");
    exit(0);
}

int main() {
    struct sigaction sa;
    sa.sa_handler = sigfpe_Handler;
    sa.sa_flags = 0;
    sigemptyset(&sa.sa_mask);

    sigaction(SIGFPE, &sa, NULL);
	
    printf("Catching SIGFPE Signal!\n");
    sleep(10); 
    printf("No SIGFPE SIgnal within 10 seconds\n");
    exit(0);
}

