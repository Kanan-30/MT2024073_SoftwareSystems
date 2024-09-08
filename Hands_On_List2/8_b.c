/* Name: Kanan Gupta
 * Roll No. MT2024073
 * Problem:
 * Write a separate program using signal system call to catch the following signals.
b. SIGINT
 * */
 
#include<signal.h>
#include<unistd.h>
#include<stdio.h>
#include<stdlib.h>


void sig_handler(int sig){
	printf("SIGINT Caught %d\n",sig);
	exit(0);
}

int main(){

	signal(SIGINT, sig_handler);
	printf("Catching SIGINT\n");
	sleep(10);
	printf("NO SIGINT within 10 seconds");

return 0;

}



