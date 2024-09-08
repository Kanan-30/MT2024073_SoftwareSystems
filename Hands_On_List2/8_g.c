/* Name: Kanan Gupta
 * Roll No. MT2024073
 * Problem:
 * Write a separate program using signal system call to catch the following signals.
g. SIGPROF (use setitimer system call)
 * */
 
#include <signal.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>

void sig_handler(int sig) {
    printf("SIGPROF Caught %d\n", sig);
    exit(0);
}

int main() {
    struct itimerval timer;

    signal(SIGPROF, sig_handler);

    printf("Catching SIGPROF\n");

    timer.it_value.tv_sec = 0;
    timer.it_value.tv_usec = 500000;
    timer.it_interval.tv_sec = 0;
    timer.it_interval.tv_usec = 0;

    setitimer(ITIMER_PROF, &timer, NULL);

    sleep(10);
    
    printf("NO SIGPROF within 10 seconds\n");

    return 0;
}




