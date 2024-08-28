/* Name: Kanan Gupta
 * Roll No. MT2024073
 * Problem: 
 * Write a program to execute ls -Rl by the following system calls
 *c. execle
 * */
 

#include <stdio.h>
#include <unistd.h>

int main() {
    
	char *envp[] = { NULL }; 

    execle("/bin/ls", "ls", "-Rl", NULL, envp);
}

