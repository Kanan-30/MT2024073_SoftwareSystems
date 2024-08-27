/* Name: Kanan Gupta
 * Roll No. MT2024073
 * Problem: 17. Write a program to simulate online ticket reservation. Implement write lock
Write a program to open a file, store a ticket number and exit. Write a separate program, to
open the file, implement write lock, read the ticket number, increment the number and print
the new ticket number then close the file.
 * */
 
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>

int main() {
    int fd;
 
    fd = open("Q16_File.txt", O_RDWR);
  
    struct{

	int train_no;
	int ticket_count;
    	} db[3];

	for(int i=0;i<3;i++) {
	     db[i].train_no = i+1;
     	     db[i].ticket_count =0;
	}

	fd = open("record.txt",O_RDWR);
	write(fd,db,sizeof(db));
	close(fd);	
}


