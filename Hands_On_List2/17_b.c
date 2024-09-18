/* Name: Kanan Gupta
 * Roll No. MT2024073
 * Problem:
 * Write a program to execute ls -l | wc.
b. use dup2
 * */
 

#include<stdio.h>
#include<unistd.h>

int main(){

	int fd[2];
	pipe(fd);

	if(!fork()){
		close(fd[0]);
		dup2(fd[1],1);
		execlp("ls", "ls" , "-l" , (char *) NULL);
	}
	else{
		close(fd[1]);
		dup2(fd[0],0);
		execlp("wc", "wc" , (char *) NULL);
	}
}
