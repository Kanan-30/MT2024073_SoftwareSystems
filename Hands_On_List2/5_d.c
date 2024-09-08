/* Name: Kanan Gupta
 * Roll No. MT2024073
 * Problem: 
 * Write a program to print the system limitation of
d. maximum number of open files
 * */
 
#include <stdio.h>
#include <unistd.h>
#include <sys/sysinfo.h>

int main() {
    long max_files = sysconf(_SC_OPEN_MAX);
    printf("Maximum number of open files: %ld\n", max_files);
}

