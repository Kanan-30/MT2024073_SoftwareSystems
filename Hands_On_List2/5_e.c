/* Name: Kanan Gupta
 * Roll No. MT2024073
 * Problem: 
 * Write a program to print the system limitation of
e. size of a page
 * */
 
#include <stdio.h>
#include <unistd.h>
#include <sys/sysinfo.h>

int main() {
    long page_size = sysconf(_SC_PAGESIZE);
    printf("Size of a page (bytes): %ld\n", page_size);
}

