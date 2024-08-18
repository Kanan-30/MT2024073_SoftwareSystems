/* Name: Kanan Gupta
 * Roll No. MT2024073
 * Problem:  Write a program to create a file and print the file descriptor value. Use creat ( ) system call
 * */
#include <stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<sys/stat.h>
#include<fcntl.h>
int main(){
	 
	char *fname = "newfile.txt";
        int filedesc = creat(fname,S_IRWXU|S_IRGRP|S_IROTH);

	if(filedesc == -1){
		printf("Error");
		return 1;
	}

	printf("File descriptor value: %d \n", filedesc);
	close(filedesc);
}	
