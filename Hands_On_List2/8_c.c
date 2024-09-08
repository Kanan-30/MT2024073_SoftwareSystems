/* Name: Kanan Gupta
 * Roll No. MT2024073
 * Problem:
 * Write a separate program using signal system call to catch the following signals.
c. SIGFPE
 * */
 
#include<signal.h>
#include<unistd.h>
#include<stdio.h>
#include<stdlib.h>


void sig_handler(int sig){
	printf("SIGFPE Caught %d\n",sig);
	exit(0);
}

int main(){

	signal(SIGFPE, sig_handler);
	printf("Catching SIGFPE\n");
	sleep(10);
	printf("NO SIGFPE within 10 seconds");

return 0;

}



