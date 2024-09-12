/* Name: Kanan Gupta
 * Roll No. MT2024073
 * Problem:Write a program to ignore a SIGINT signal then reset the default action of the SIGINT
signal - Use signal system call. 
 * */
 
#include<unistd.h>
#include<stdio.h>
#include<stdlib.h>
#include<signal.h>


void sigint_Handler(int sig) {
    printf("Caught SIGINT signal\n");
}

int main() {
    
    signal(SIGINT, sigint_Handler);

    printf("Catching SIGINT!\n\n");
    sleep(5); 

    
    signal(SIGINT, SIG_DFL);

    printf("SIGINT reset to default.\n");

    while(1){
	    pause();
    }
    return 0;
}

