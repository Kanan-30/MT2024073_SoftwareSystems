/* Name: Kanan Gupta
 * Roll No. MT2024073
 * Problem: 
 * Write a program to print the system limitation of
a. maximum length of the arguments to the exec family of functions.
 * */
 

#include <stdio.h>
#include <unistd.h>
#include <sys/sysinfo.h>

int main() {
    long max_arg_length = sysconf(_SC_ARG_MAX);
    printf("Maximum length of arguments to exec family of functions: %ld\n", max_arg_length);
}
