/* Name: Kanan Gupta
 * Roll No. MT2024073
 * Problem:
 * Write a separate program using signal system call to catch the following signals.
a. SIGSEGV
 * */
 
#include<signal.h>
#include<unistd.h>
#include<stdio.h>
#include<stdlib.h>


void sig_handler(int sig){
	printf("SIGSEGV Caught %d\n",sig);
	exit(0);
}

int main(){
	int i,j;
	signal(SIGSEGV, sig_handler);
	printf("Catching SIGSEGV\n");
	scanf("%d",i);
	sleep(10);
	printf("NO SIGSEGV within 10 seconds\n");

//return 0;

}



