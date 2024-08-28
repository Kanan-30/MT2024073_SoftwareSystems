/* Name: Kanan Gupta
 * Roll No. MT2024073
 * Problem: Write a program to get maximum and minimum real time priority.
 * */
 

#include <stdio.h>
#include <stdlib.h>
#include <sched.h>

int main() {
    int min_priority_fifo, max_priority_fifo;
    int min_priority_rr, max_priority_rr;

    min_priority_fifo = sched_get_priority_min(SCHED_FIFO);
    max_priority_fifo = sched_get_priority_max(SCHED_FIFO);

    min_priority_rr = sched_get_priority_min(SCHED_RR);
    max_priority_rr = sched_get_priority_max(SCHED_RR);

    printf("Real-time scheduling priorities for SCHED_FIFO:\n");
    printf("Minimum priority: %d\n", min_priority_fifo);
    printf("Maximum priority: %d\n", max_priority_fifo);

    printf("Real-time scheduling priorities for SCHED_RR:\n");
    printf("Minimum priority: %d\n", min_priority_rr);
    printf("Maximum priority: %d\n", max_priority_rr);

    return 0;
}


