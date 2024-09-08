/* Name: Kanan Gupta
 * Roll No. MT2024073
 * Problem:
 * Write a separate program using signal system call to catch the following signals.
e.SIGALRM
 * */
 
#include <signal.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>

void sig_handler(int sig) {
    printf("SIGALRM Caught %d\n", sig);
    exit(0);
}

int main() {
    struct itimerval timer;

    signal(SIGALRM, sig_handler);

    printf("Catching SIGALRM\n");

    timer.it_value.tv_sec = 2;
    timer.it_value.tv_usec = 0;
    timer.it_interval.tv_sec = 0;
    timer.it_interval.tv_usec = 0;

    setitimer(ITIMER_REAL, &timer, NULL);

    sleep(10);

    return 0;
}



