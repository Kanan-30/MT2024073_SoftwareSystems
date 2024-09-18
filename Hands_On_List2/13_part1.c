/* Name: Kanan Gupta
 * Roll No. MT2024073
 * Problem: 
 * Write two programs: first program is waiting to catch SIGSTOP signal, the second program
will send the signal (using kill system call). Find out whether the first program is able to catch
the signal or not.
 * */
 

#include<signal.h>
#include<unistd.h>
#include<stdio.h>
#include<stdlib.h>


void sig_handler(int sig){
        printf("SIGSTOP Caught %d\n",sig);
        exit(0);
}

int main(){

        signal(SIGSTOP, sig_handler);
        printf("Catching SIGSTOP\n");
        sleep(20);
        printf("NO SIGSTOP within 20 seconds");

return 0;

}

