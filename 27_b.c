/* Name: Kanan Gupta
 * Roll No. MT2024073
 * Problem: 
 * Write a program to execute ls -Rl by the following system calls
 *b. execlp
 * */
 

#include <stdio.h>
#include <unistd.h>

int main() {
    execlp("ls", "ls", "-Rl", NULL);
}

