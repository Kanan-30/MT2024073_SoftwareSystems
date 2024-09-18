/* Name: Kanan Gupta
 * Roll No. MT2024073
 * Problem: Write a program to wait for data to be written into FIFO within 10 seconds, use select
system call with FIFO. 
 * */
 
#include<stdio.h>
#include<unistd.h>
#include<sys/time.h>
#include<fcntl.h>
#include<stdlib.h>

int main(){

	int fd,retval;
	char buff[100];
	fd_set rfds;
	struct timeval tv;

	fd = open("myfifo1", O_RDONLY);
	FD_ZERO(&rfds);
	FD_SET(fd,&rfds);

	tv.tv_sec = 10;
	tv.tv_usec=0;

	retval = select(fd+1,&rfds,NULL,NULL,&tv);
	if(retval)
		printf("Data is available\n");
	else{
		printf("Data not available within 10 seconds \n");
		exit(0);
	}
	read(fd,buff,sizeof(buff));
	printf("FIFO Text: %s\n",buff);
}
