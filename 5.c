/* Name: Kanan Gupta
 * Roll No. MT2024073
 * Problem: Write a program to create five new files with infinite loop. Execute the program in the background
and check the file descriptor table at /proc/pid/fd.
 * */
 
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>

int main() {
    int i;
    char filename[20];
    
    while (1) {
        for (i = 0; i < 5; i++) {
            snprintf(filename, sizeof(filename), "file%d.txt", i);
            open(filename, O_CREAT | O_WRONLY | O_APPEND, 0644);
        }
        sleep(3);
    }

    return 0;
}


