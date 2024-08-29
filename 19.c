/* Name: Kanan Gupta
 * Roll No. MT2024073
 * Problem: Write a program to find out time taken to execute getpid system call. Use time stamp counter.
 * */
 

#include <stdio.h>
#include <unistd.h>
#include<sys/time.h>

unsigned long long rdtsc() {
    unsigned long long dst;
    __asm__ __volatile__ ("rdtsc" : "=A" (dst));
    }
int main() {
    unsigned long long start, end, time;
    
    pid_t pid; 

    // before getpid()
    start = rdtsc();

    pid = getpid();

    // after getpid()
    end = rdtsc();

    time = (end - start)/2.09;

    
    printf("Time taken by getpid(): %llu clock cycles\n", time);

    return 0;
}



