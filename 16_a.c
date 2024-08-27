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

int main() {
    int fd;
 
    fd = open("Q16_File.txt", O_RDWR);
  
    struct flock lock;
    lock.l_type = F_WRLCK;  
    lock.l_whence = SEEK_SET;
    lock.l_start = 0;
    lock.l_len = 0;  // for mandatory lock set len =0
    lock.l_pid = getpid();

    printf("Outside the critical section\n");

    fcntl(fd,F_SETLKW,&lock);

    printf("File has been Write locked - you cannot Write or read!\n You are inside the critical section\n");
  
    printf("IF you want to release the lock, press Enter\n");
    getchar();

    
    lock.l_type = F_UNLCK;  
    fcntl(fd,F_SETLK,&lock);
    printf("Lock released.\n");

    close(fd);
    
}


