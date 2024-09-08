/* Name: Kanan Gupta
 * Roll No. MT2024073
 * Problem: 
 * Write a program to print the system limitation of
b. maximum number of simultaneous process per user id.
 * */
 
#include <stdio.h>
#include <unistd.h>
#include <sys/sysinfo.h>

int main() {
    long max_procs = sysconf(_SC_CHILD_MAX);
    printf("Maximum number of simultaneous process per user id: %ld\n", max_procs);

}

