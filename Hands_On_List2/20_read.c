/* Name: Kanan Gupta
 * Roll No. MT2024073
 * Problem: 
 * Write two programs so that both can communicate by FIFO -Use one way communication.
 * */
 
#include<stdio.h>
#include<unistd.h>
#include<fcntl.h>

int main(){
	int fd;
	char buff[100];

	fd = open("myfifo1",O_RDONLY);
	read(fd,buff,sizeof(buff));
	printf("Text from FIFO: %s\n", buff);
}

