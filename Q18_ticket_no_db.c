/* Name: Kanan Gupta
 * Roll No. MT2024073
 * Problem: 18. Write a program to perform Record locking.
a. Implement write lock
b. Implement read lock
Create three records in a file. Whenever you access a particular record, first lock it then modify/access
to avoid race condition.
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


