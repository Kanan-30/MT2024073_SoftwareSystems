/* Name: Kanan Gupta
 * Roll No. MT2024073
 * Problem: Write a program to wait for a STDIN for 10 seconds using select. Write a proper print statement to
verify whether the data is available within 10 seconds or not (check in $man 2 select).
 * */
 

/*
 * #include <sys/select.h>
 *
 *FD_SET() -  This macro adds the file descriptor fd to set.  Adding a file descriptor that is already present in the set is a no-op, and does not produce an error.
FD_ZERO() - This macro clears (removes all file descriptors from) set.  It should be employed as the first step in initializing a file descriptor set.

    fd_set ()- set that select will monitor.
    FD_ZERO(&read): to initialize the file descriptor - no file descriptors.
    FD_SET(STDIN_FILENO, &read): STDIN_FILENO - 0-  standard input
   
        -1 - error
        0 - timeout 
        >0 - ready 


 */
#include <sys/select.h>
#include<unistd.h>
#include<fcntl.h>
#include<stdlib.h>
#include<stdio.h>


int main(){

	fd_set read;
	struct timeval wait_time;
	int result;

	FD_ZERO (&read);
	FD_SET(STDIN_FILENO,&read);

	wait_time.tv_sec=10;
	wait_time.tv_usec=0;

	result = select(STDIN_FILENO+1, &read, NULL, NULL, &wait_time);

	if(result==-1){
		printf("Error\n");
	}
	else if (result>0){
		printf("Data avaialble within 10 seconds!\n ");
	}
	else{
		printf("Data not available within 10 seconds\n");
	}
}

