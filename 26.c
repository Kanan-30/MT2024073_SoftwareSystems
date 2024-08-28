/* Name: Kanan Gupta
 * Roll No. MT2024073
 * Problem:
 * Write a program to execute an executable program.
a. use some executable program
b. pass some input to an executable program. (for example execute an executable of $./a.out name)
 * */
 
#include <stdio.h>
#include <unistd.h>

int main() {
  
    char *execprogram = "./type_of_file";  
    char *arg1 = "pipe2";        
    char *args[] = {execprogram, arg1, NULL};  

    
    execvp(execprogram, args);

}


