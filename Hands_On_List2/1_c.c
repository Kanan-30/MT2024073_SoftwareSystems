/* Name: Kanan Gupta
 * Roll No. MT2024073
 * Problem: 
 * Write a separate program (for each time domain) to set a interval timer in 10sec and
10micro second
a. ITIMER_REAL
b. ITIMER_VIRTUAL
c. ITIMER_PROF
 * */


#include <sys/time.h>
#include <stdio.h>
#include <signal.h>
#include <unistd.h>

int main(){

struct itimerval timer, current_timer;

timer.it_value.tv_sec = 10;
timer.it_value.tv_usec = 10;

timer.it_interval.tv_sec = 10;
timer.it_interval.tv_usec = 10;

setitimer(ITIMER_PROF, &timer, NULL);

for (long i = 0; i < 100000000; i++) {
	// usermode work
    }

sleep(2); // kernel mode work
printf("Profiling Timer Set\n");
getitimer(ITIMER_PROF, &current_timer);

printf("Time remaining: %ld sec, %ld usec\n", current_timer.it_value.tv_sec, current_timer.it_value.tv_usec);
printf("Interval: %ld sec, %ld usec\n", current_timer.it_interval.tv_sec, current_timer.it_interval.tv_usec);

}
