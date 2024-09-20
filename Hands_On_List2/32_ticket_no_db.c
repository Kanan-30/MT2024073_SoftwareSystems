/* Name: Kanan Gupta
 * Roll No. MT2024073
 * Problem: Write a program to implement semaphore to protect any critical section.
a. rewrite the ticket number creation program using semaphore
b. protect shared memory from concurrent write access
c. protect multiple pseudo resources ( may be two) using counting semaphore
d. remove the created semaphore
 * */
 
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>

int main() {
    int fd;
  
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


