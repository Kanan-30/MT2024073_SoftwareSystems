/* Name: Kanan Gupta
 * Roll No. MT2024073
 * Problem: 
 * Write a program to execute ls -Rl by the following system calls
 *a. execl
 * */
 

#include <stdio.h>
#include <unistd.h>

int main() {
    execl("/bin/ls", "ls", "-Rl", NULL);
}

