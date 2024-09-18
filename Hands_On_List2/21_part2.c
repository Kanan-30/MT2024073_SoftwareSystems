/* Name: Kanan Gupta
 * Roll No. MT2024073
 * Problem: 
 * Write two programs so that both can communicate by FIFO -Use two way communication.
 * */
 
#include<stdio.h>
#include<unistd.h>
#include<fcntl.h>

int main(){
	int fd1,fd2;
	char buff1[100],buff2[100];

	fd1 = open("myfifo1",O_RDONLY);
	fd2 = open("myfifo2",O_WRONLY);

	read(fd1,buff1,sizeof(buff1));
        printf("Text from FIFO: %s\n", buff1);


	printf("Enter Text: ");
	scanf(" %[^\n]", buff2);
	write(fd2,buff2,sizeof(buff2));
}

