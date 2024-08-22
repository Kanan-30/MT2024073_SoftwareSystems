#include <unistd.h>
#include <fcntl.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

int main() {
    int fd1, fd2, fd3, fd4;
    ssize_t bytes_written;

    fd1 = open("Q11_FIle1.txt", O_RDWR | O_APPEND, S_IRUSR | S_IWUSR);

    char orgString[] = "Original Update!\n";
    bytes_written = write(fd1, orgString, strlen(orgString));

    fd2 = dup(fd1);

    char dupString[] = "Dup Method 1!\n";

    bytes_written = write(fd2, dupString, strlen(dupString));

    fd3 = dup2(fd1, 10);

    char dup1String[] = "Dup Method 2!\n";
    bytes_written = write(fd3, dup1String, strlen(dup1String));


    fd4 = fcntl(fd1, F_DUPFD, 20);

    char dup2String[] = "Dup Method 3!\n";
    bytes_written = write(fd4, dup2String, strlen(dup2String));

    close(fd1);
    close(fd2);
    close(fd3);
    close(fd4);
}

