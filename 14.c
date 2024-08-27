/* Name: Kanan Gupta
 * Roll No. MT2024073
 * Problem: Write a program to find the type of a file.
a. Input should be taken from command line.
b. program should be able to identify any type of a file.
 * */
 
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>

int main(int argc, char *argv[]) {
    struct stat file_stat;

    if (argc != 2) {
        printf("PLease enter 2nd argument that is file or file path: \n");
        return 1;
    }

    if (stat(argv[1], &file_stat) == -1) {
        perror("stat");
        return 1;
    }

    
    if (S_ISREG(file_stat.st_mode)) {
        printf("'%s' is a regular file.\n", argv[1]);
    } else if (S_ISDIR(file_stat.st_mode)) {
        printf("'%s' is a directory.\n", argv[1]);
    } else if (S_ISCHR(file_stat.st_mode)) {
        printf("'%s' is a character device.\n", argv[1]);
    } else if (S_ISBLK(file_stat.st_mode)) {
        printf("'%s' is a block device.\n", argv[1]);
    } else if (S_ISFIFO(file_stat.st_mode)) {
        printf("'%s' is a FIFO (named pipe).\n", argv[1]);
    } else if (S_ISLNK(file_stat.st_mode)) {
        printf("'%s' is a symbolic link.\n", argv[1]);
    } else if (S_ISSOCK(file_stat.st_mode)) {
        printf("'%s' is a socket.\n", argv[1]);
    } else {
        printf("'%s' is of unknown type.\n", argv[1]);
    }

    return 0;
}


