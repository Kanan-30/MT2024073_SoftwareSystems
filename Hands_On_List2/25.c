/* Name: Kanan Gupta
 * Roll No. MT2024073
 * Problem: 
 * Write a program to print a message queue's (use msqid_ds and ipc_perm structures)
a. access permission
b. uid, gid
c. time of last message sent and received
d. time of last change in the message queue
d. size of the queue
f. number of messages in the queue
g. maximum number of bytes allowed
h. pid of the msgsnd and msgrcv
 * */
 
#include <stdio.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <time.h>

int main() {
    key_t key;
    int msgid;
    struct msqid_ds buff;

    key = ftok(".", 'a');
    msgid = msgget(key, 0666);

    msgctl(msgid, IPC_STAT, &buff);

    printf("Access Permissions: %o\n", buff.msg_perm.mode & 0777);
    printf("UID: %u\n", buff.msg_perm.uid);
    printf("GID: %u\n", buff.msg_perm.gid);
    printf("Time of last message sent: %s", ctime(&buff.msg_stime));
    printf("Time of last message received: %s", ctime(&buff.msg_rtime));
    printf("Time of last change in message queue: %s", ctime(&buff.msg_ctime));
    printf("Size of queue: %lu\n", buff.__msg_cbytes);
    printf("Number of messages in queue: %lu\n", buff.msg_qnum);
    printf("Maximum number of bytes allowed in queue: %lu\n", buff.msg_qbytes);
    printf("PID of last msgsnd: %d\n", buff.msg_lspid);
    printf("PID of last msgrcv: %d\n", buff.msg_lrpid);

    return 0;
}


