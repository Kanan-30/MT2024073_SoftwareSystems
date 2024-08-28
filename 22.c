/* Name: Kanan Gupta
 * Roll No. MT2024073
 * Problem: Write a program, open a file, call fork, and then write to the file by both the child as well as the
parent processes. Check output of the file.
 * */
 
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <sys/wait.h>

int main() {
    
    int fd = open("Q22_File.txt", O_WRONLY | O_CREAT | O_TRUNC, 0644);

    
    pid_t pid = fork();

    if (pid == 0) { 
        const char *child_text = "Added text by child process!\n";
        write(fd, child_text, strlen(child_text));
    } else { 
        const char *parent_text = "Added text by parent process!\n";
        write(fd, parent_text, strlen(parent_text));

      
        wait(NULL);
    }

    close(fd);
}


