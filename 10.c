/* Name: Kanan Gupta
 * Roll No. MT2024073
 * Problem: Write a program to open a file with read write mode, write 10 bytes, move the file pointer by 10 bytes (use lseek) and write again 10 bytes.
a. check the return value of lseek
b. open the file with od and check the empty spaces in between the data.
 * */
 

#include<unistd.h>
#include<fcntl.h>
#include<stdlib.h>
#include<stdio.h>


int main(){

	int fd;
	ssize_t bytes_written; 
	off_t offset;

	char data1[] = "ABCDEFGHIJ";
	char data2[] = "abcdefghij";

	fd = open("Q10.txt",O_RDWR | O_CREAT);

	bytes_written = write(fd,data1,10);
	
	printf("First 10 bytes written \n");

	offset = lseek(fd, 10,SEEK_CUR); 

	printf("Lseek moved the filepointer to: %ld\n", offset);

	bytes_written = write(fd,data2,10);

	printf("Next 10 bytes written\n");

	 close(fd);
}

