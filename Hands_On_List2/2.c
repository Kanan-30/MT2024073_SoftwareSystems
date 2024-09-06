/* Name: Kanan Gupta
 * Roll No. MT2024073
 * Problem: 
 * Write a program to print the system resource limits. Use getrlimit system call.
 * */
 
#include <stdio.h>
#include <stdlib.h>
#include <sys/resource.h>

int main() {
    struct rlimit rlim;

    printf("System Resource Limits:\n");

    getrlimit(RLIMIT_DATA, &rlim);
    printf("Maximum size of a process's data area (RLIMIT_DATA):\n");
    printf("  Soft limit: %ld bytes\n", rlim.rlim_cur);
    printf("  Hard limit: %ld bytes\n", rlim.rlim_max);

    getrlimit(RLIMIT_STACK, &rlim);
    printf("Maximum stack size (RLIMIT_STACK):\n");
    printf("  Soft limit: %ld bytes\n", rlim.rlim_cur);
    printf("  Hard limit: %ld bytes\n", rlim.rlim_max);

    getrlimit(RLIMIT_NOFILE, &rlim);
    printf("Maximum number of open files (RLIMIT_NOFILE):\n");
    printf("  Soft limit: %ld\n", rlim.rlim_cur);
    printf("  Hard limit: %ld\n", rlim.rlim_max);

    getrlimit(RLIMIT_NPROC, &rlim);
    printf("Maximum number of processes (RLIMIT_NPROC):\n");
    printf("  Soft limit: %ld\n", rlim.rlim_cur);
    printf("  Hard limit: %ld\n", rlim.rlim_max);

    getrlimit(RLIMIT_CPU, &rlim);
    printf("Maximum amount of CPU time (RLIMIT_CPU):\n");
    printf("  Soft limit: %ld seconds\n", rlim.rlim_cur);
    printf("  Hard limit: %ld seconds\n", rlim.rlim_max);

    return 0;
}




