/* Name: Kanan Gupta
 * Roll No. MT2024073
 * Problem:
 * Write a separate program using signal system call to catch the following signals.
d. SIGALRM
 * */
 
#include<signal.h>
#include<unistd.h>
#include<stdio.h>
#include<stdlib.h>


void sig_handler(int sig){
	printf("SIGALRM Caught %d\n",sig);
	exit(0);
}

int main(){

	signal(SIGALRM, sig_handler);
	printf("Catching SIGALRM\n");
	alarm(2);
	sleep(10);


return 0;

}



