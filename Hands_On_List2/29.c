/* Name: Kanan Gupta
 * Roll No. MT2024073
 * Problem: 
 * Write a program to remove the message queue.
 * */
 
#include <stdio.h>
#include <sys/ipc.h>
#include <sys/msg.h>

int main() {
    key_t key;
    int msqid;

    key = ftok(".", 'a');
    msqid = msgget(key, 0);

    msgctl(msqid, IPC_RMID, NULL);

    return 0;
}


