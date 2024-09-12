/* Name: Kanan Gupta
 * Roll No. MT2024073
 * Problem: Write a program to ignore a SIGINT signal then reset the default action of the SIGINT signal -
use sigaction system call
 * */
 

#include<stdio.h>
#include<unistd.h>
#include<stdlib.h>
#include<signal.h>

void sigint_Handler(int sig){
	printf("Caught SIGINT SIgnal\n");
}

int main(){
	
	struct sigaction sa;
	sa.sa_handler = sigint_Handler;
	sa.sa_flags = 0;
	sigemptyset(&sa.sa_mask);
        
	sigaction(SIGINT, &sa, NULL);
      
        printf("Catching SIGINT Signal!\n");
	sleep(5);

	sa.sa_handler = SIG_DFL;
	sigaction(SIGINT, &sa, NULL);

	printf("SIGINT reset to default\n");

	sleep(10);

	exit(0);
}
