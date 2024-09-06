/* Name: Kanan Gupta
 * Roll No. MT2024073
 * Problem: 
 * Write a program to set (any one) system resource limit. Use setrlimit system call.
 * */
 

#include <stdio.h>
#include <stdlib.h>
#include <sys/resource.h>

int main() {
    struct rlimit rlim;

    printf("Old limits:\n");
    getrlimit(RLIMIT_NOFILE, &rlim);
    printf("  Soft limit: %ld\n", rlim.rlim_cur);
    printf("  Hard limit: %ld\n", rlim.rlim_max);

    
    rlim.rlim_cur = 1024;  
    rlim.rlim_max = 2048; 
    setrlimit(RLIMIT_NOFILE, &rlim);

  
    getrlimit(RLIMIT_NOFILE, &rlim);
    printf("New limits:\n");
    printf("  Soft limit: %ld\n", rlim.rlim_cur);
    printf("  Hard limit: %ld\n", rlim.rlim_max);

}
