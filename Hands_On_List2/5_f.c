/* Name: Kanan Gupta
 * Roll No. MT2024073
 * Problem: 
 * Write a program to print the system limitation of
f. total number of pages in the physical memory
 * */
 
#include <stdio.h>
#include <unistd.h>
#include <sys/sysinfo.h>

int main() {
   long page_size = sysconf(_SC_PAGESIZE);
   struct sysinfo sys_info;
    sysinfo(&sys_info);
    printf("Total number of pages in physical memory: %ld\n", sys_info.totalram / page_size);
}

