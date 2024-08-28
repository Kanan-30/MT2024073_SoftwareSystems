/* Name: Kanan Gupta
 * Roll No. MT2024073
 * Problem: 
 * Write a program to execute ls -Rl by the following system calls
 *e. execp
 * */
 

#include <stdio.h>
#include <unistd.h>

int main() {
    char *args[] = { "ls", "-Rl", NULL };

    execvp("ls", args);
}

