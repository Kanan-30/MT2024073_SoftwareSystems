/* Name: Kanan Gupta
 * Roll No. MT2024073
 * Problem: Write a program to find out time taken to execute getpid system call. Use time stamp counter.
 * */
 

#include <stdio.h>
#include <unistd.h>
#include <x86intrin.h>  // for __rdtsc()

int main() {
    unsigned long long start, end, cycles;
    pid_t pid;

    // before getpid()
    start = __rdtsc();

    pid = getpid();

    // after getpid()
    end = __rdtsc();

    cycles = end - start;

    // Print the result
    printf("Time taken by getpid(): %llu clock cycles\n", cycles);

    return 0;
}



