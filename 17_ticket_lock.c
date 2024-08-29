/* Name: Kanan Gupta
 * Roll No. MT2024073
 * Problem: Write a program to simulate online ticket reservation. Implement write lock
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

	struct{
		int ticket_no;
	}db;

    int fd;
    fd = open("db",O_RDWR);
    read(fd,&db,sizeof(db));
    struct flock lock;
    lock.l_type = F_WRLCK;  
    lock.l_whence = SEEK_SET;
    lock.l_start = 0;
    lock.l_len = 0;  // for mandatory lock set len =0
    lock.l_pid = getpid();

    printf("Outside the critical section\n");

    fcntl(fd,F_SETLKW,&lock);

    printf("Current Ticket no: %d\n",db.ticket_no);
    db.ticket_no++;
    lseek(fd,0L,SEEK_SET);
  
    printf("Inside the critical section\n");
    printf("IF you want to release the lock, press Enter\n");

    write(fd,&db,sizeof(db));
    getchar();

    
    lock.l_type = F_UNLCK;  
    fcntl(fd,F_SETLK,&lock);
    printf("Lock released.\n");

    close(fd);
    
}


