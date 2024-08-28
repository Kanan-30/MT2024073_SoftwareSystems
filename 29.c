/* Name: Kanan Gupta
 * Roll No. MT2024073
 * Problem: Write a program to get scheduling policy and modify the scheduling policy (SCHED_FIFO,
SCHED_RR).
 * */
 
#include <stdio.h>
#include <stdlib.h>
#include <sched.h>

int main() {
    int policy;
    struct sched_param param;

    policy = sched_getscheduler(0);
    printf("The current scheduling policy is: ");
    switch (policy) {
        case SCHED_FIFO: printf("SCHED_FIFO\n"); break;
        case SCHED_RR:   printf("SCHED_RR\n"); break;
        case SCHED_OTHER:printf("SCHED_OTHER\n"); break;
        default:         printf("Unknown policy\n"); break;
    }

    param.sched_priority = 10;
    sched_setscheduler(0, SCHED_FIFO, &param);

    policy = sched_getscheduler(0);
    printf("Scheduling policy after setting to SCHED_FIFO: ");
    switch (policy) {
        case SCHED_FIFO: printf("SCHED_FIFO\n"); break;
        case SCHED_RR:   printf("SCHED_RR\n"); break;
        case SCHED_OTHER:printf("SCHED_OTHER\n"); break;
        default:         printf("Unknown policy\n"); break;
    }

    param.sched_priority = 20;
    sched_setscheduler(0, SCHED_RR, &param);

    policy = sched_getscheduler(0);
    printf("Scheduling policy after setting to SCHED_RR: ");
    switch (policy) {
        case SCHED_FIFO: printf("SCHED_FIFO\n"); break;
        case SCHED_RR:   printf("SCHED_RR\n"); break;
        case SCHED_OTHER:printf("SCHED_OTHER\n"); break;
        default:         printf("Unknown policy\n"); break;
    }

    return 0;
}


