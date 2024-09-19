/* Name: Kanan Gupta
 * Roll No. MT2024073
 * Problem: 
 Write a program to print the maximum number of files can be opened within a process and
size of a pipe (circular buffer).
 * */
 
#include <stdio.h>
#include <unistd.h>
#include <limits.h>
#include <fcntl.h>
#include <stdlib.h>

int main() {
    long max_no_open_files = sysconf(_SC_OPEN_MAX);
    printf("Maximum number of open files within a process: %ld\n", max_no_open_files);

    int fd[2];
    pipe(fd);

    //int pipe_size = fcntl(fd[0], F_GETPIPE_SZ);
    int pipe_size = fcntl(fd[0], _PC_PIPE_BUF);
    
    printf("Pipe size (circular buffer size): %d bytes\n", pipe_size);

    close(fd[0]);
    close(fd[1]);

    return 0;
}


