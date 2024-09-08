/* Name: Kanan Gupta
 * Roll No. MT2024073
 * Problem: 
 * Write a program to print the system limitation of
g. number of currently available pages in the physical memory.
 * */
 
#include <stdio.h>
#include <unistd.h>
#include <sys/sysinfo.h>

int main() {
    
    long page_size = sysconf(_SC_PAGESIZE);
    struct sysinfo sys_info;
    sysinfo(&sys_info);
     printf("Number of currently available pages in the  physical memory: %ld\n", sys_info.freeram / page_size);
}

