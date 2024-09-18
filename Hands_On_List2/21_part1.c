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

	fd1 = open("myfifo1",O_WRONLY);
	fd2 = open("myfifo2",O_RDONLY);
	printf("Enter Text: ");
	scanf(" %[^\n]", buff1);
	write(fd1,buff1,sizeof(buff1));

	read(fd2,buff2,sizeof(buff2));
        printf("Text from FIFO: %s\n", buff2);

}

