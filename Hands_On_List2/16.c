/* Name: Kanan Gupta
 * Roll No. MT2024073
 * Problem: Write a program to send and receive data from parent to child vice versa. Use two way
communication.
 * */
 
#include<stdio.h>
#include<unistd.h>
#include<sys/wait.h>

int main(){
	char buff1[100], buff2[100];
	int fd1[2],fd2[2];

	pipe(fd1);
	pipe(fd2);

	if(!fork()){
		close(fd1[0]);
		printf("ENter message to Parent: \n");
		scanf(" %[^\n]", buff1);
		write(fd1[1], buff1, sizeof(buff1));
		close(fd2[1]);
		read(fd2[0], buff2,sizeof(buff2));
		printf("Message FRom Parent: %s\n",buff2);
	}
	else{
		close(fd1[1]);
		read(fd1[0], buff1,sizeof(buff1));
                printf("Message FRom Child : %s\n",buff1);
		close(fd2[0]);
		printf("ENter message to Child: \n");
                scanf(" %[^\n]", buff2);
                write(fd2[1], buff2, sizeof(buff2));
	}
	wait(0);
}



