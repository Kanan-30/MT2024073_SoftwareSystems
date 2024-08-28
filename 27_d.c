/* Name: Kanan Gupta
 * Roll No. MT2024073
 * Problem: 
 * Write a program to execute ls -Rl by the following system calls
 *a. execv
 * */
 

#include <stdio.h>
#include <unistd.h>

int main() {
    char *args[] = { "ls", "-Rl", NULL };

    execv("/bin/ls", args);

}

