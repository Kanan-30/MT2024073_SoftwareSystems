/* Name: Kanan Gupta
 * Roll No. MT2024073
 * Problem:Write a program to display the environmental variable of the user (use environ). 
 * */
 
#include <stdio.h>
#include <stdlib.h>

extern char **environ;

int main() {
    char **envVar = environ;

    while (*envVar) {
        printf("%s\n", *envVar);
        envVar++;
    }

}



