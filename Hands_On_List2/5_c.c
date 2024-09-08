/* Name: Kanan Gupta
 * Roll No. MT2024073
 * Problem: 
 * Write a program to print the system limitation of
c. number of clock ticks (jiffy) per second 
 * */
 
#include <stdio.h>
#include <unistd.h>
#include <sys/sysinfo.h>

int main() {
    long clock_ticks = sysconf(_SC_CLK_TCK);
    printf("Number of clock ticks (jiffy) per second: %ld\n", clock_ticks);
}

