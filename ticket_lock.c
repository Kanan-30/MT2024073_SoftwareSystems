/* Name: Kanan Gupta
 * Roll No. MT2024073
 * Problem: Write a program to perform mandatory locking.
a. Implement write lock
 * */
 
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>

struct {
	int train_no;
	int ticket_count;
} db;

int main() {
    int fd, input;
 
    fd = open("record.txt", O_RDWR);
    printf("Select train number in which you want to book a ticket {1,2,3}\n");
    scanf("%d",&input);
  
    struct flock lock;
    lock.l_type = F_WRLCK;  
    lock.l_whence = SEEK_SET;
    lock.l_start = (input-1)*sizeof(db);
    lock.l_len = sizeof(db);
    lock.l_pid = getpid();

    lseek(fd, (input-1)*sizeof(db),SEEK_SET);
    read(fd,&db,sizeof(db));
    printf("Outside the train booking page.....!!\n");

    fcntl(fd,F_SETLKW,&lock);

    printf("Ticket NUmber: %d\n", db.ticket_count);
    db.ticket_count++;

    lseek(fd,-1*sizeof(db),SEEK_CUR);
    write(fd,&db,sizeof(db));
  
    printf("To book ticket, press Enter\n");
    getchar();
    getchar();
    lock.l_type = F_UNLCK;  
    fcntl(fd,F_SETLK,&lock);
    printf("Ticket Booked!!.\n");

    close(fd);
    
}


