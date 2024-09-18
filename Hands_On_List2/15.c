/* Name: Kanan Gupta
 * Roll No. MT2024073
 * Problem: Write a simple program to send some data from parent to the child process.
 * */


#include<stdio.h>
#include<unistd.h>
#include<sys/wait.h>

int main(){
	char buff[100];
	int fd[2];
	pipe(fd);

	if(fork()){ 
		//parent writing messaging so closing the read end of pipe
		close(fd[0]);
		printf("Enter message to be sent to child: \n");
		scanf(" %[^\n]",buff);
		write(fd[1],buff,sizeof(buff));
	}
	else{
		//child reading the message so closing the write end of the pipe
		close(fd[1]);
		read(fd[0],buff,sizeof(buff));
		printf("Message received from parent: %s\n",buff);
	}
	wait(0);
}


