/* Name: Kanan Gupta
 * Roll No. MT2024073
 * Problem:
 * Write a separate program using signal system call to catch the following signals.
f.SIGVTALRM (use setitimer system call)
 * */
 
#include <signal.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>

void sig_handler(int sig) {
    printf("SIGVTALRM Caught %d\n", sig);
    exit(0);
}

int main() {
    struct itimerval timer;

    signal(SIGVTALRM, sig_handler);

    printf("Catching SIGVTALRM\n");

    timer.it_value.tv_sec = 2;
    timer.it_value.tv_usec = 0;
    timer.it_interval.tv_sec = 1;
    timer.it_interval.tv_usec = 0;

    setitimer(ITIMER_VIRTUAL, &timer, NULL);

    sleep(10);
    
    printf("NO SIGVTALRM within 10 seconds\n");

    return 0;
}




