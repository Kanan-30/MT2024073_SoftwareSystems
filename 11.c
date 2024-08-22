/* Name: Kanan Gupta
 * Roll No. MT2024073
 * Problem: Write a program to open a file, duplicate the file descriptor and append the file with both the
descriptors and check whether the file is updated properly or not.
a. use dup
b. use dup2
c. use fcntl
 * */
 

#include<unistd.h>
#include<fcntl.h>
#include<stdlib.h>
#include<stdio.h>


int main(){

	int fd1,fd2,fd3,fd4;

	fd1 = open("Q11_FIle1.txt",O_RDONLY|O_APPEND);

	if(fd1==-1){

		perror("file doesnot exist");
	}

	fd2 = dup(fd1);

	fd3 = dup2(fd1,10);
	
	fd4 = fcntl(fd1,F_DUPFD,20);

	close(fd1);
	close(fd2);
	close(fd3);
	close(fd4);

}

