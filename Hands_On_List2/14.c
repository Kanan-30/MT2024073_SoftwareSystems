/* Name: Kanan Gupta
 * Roll No. MT2024073
 * Problem:Write a simple program to create a pipe, write to the pipe, read from pipe and display on
the monitor. 
 * */
 
#include<stdio.h>
#include<unistd.h>

int main(){
	int fd[2];
	char buff[20];
	pipe(fd);

	write(fd[1],"hello THis is Qn 14\n" ,20);
	read(fd[0],buff,sizeof(buff));
	printf("Read from pipe: %s\n",buff);
}
