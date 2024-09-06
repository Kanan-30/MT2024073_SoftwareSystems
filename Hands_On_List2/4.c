/* Name: Kanan Gupta
 * Roll No. MT2024073
 * Problem: Write a program to measure how much time is taken to execute 100 getppid ( )
system call. Use time stamp counter.
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

    // before getpid()
    start = rdtsc();
   
    for(int i =0; i<100;i++){
   	 getppid();
    }

    // after getpid()
    end = rdtsc();

    time = (end - start)/2.09;

    
    printf("Time taken by getpid(): %llu clock cycles\n", time);

    return 0;
}



