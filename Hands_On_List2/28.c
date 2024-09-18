/* Name: Kanan Gupta
 * Roll No. MT2024073
 * Problem: 
 * Write a program to change the exiting message queue permission. (use msqid_ds structure)
 * */
 
#include <stdio.h>
#include <sys/ipc.h>
#include <sys/msg.h>

int main() {
    key_t key;
    int msgid;
    struct msqid_ds buff;

    key = ftok(".", 'a');
    msgid = msgget(key, 0);

    msgctl(msgid, IPC_STAT, &buff);

    printf("Current permissions: %o\n", buff.msg_perm.mode & 0777);

    buff.msg_perm.mode = (buff.msg_perm.mode & ~0777) | 0644;

    msgctl(msgid, IPC_SET, &buff);

    msgctl(msgid, IPC_STAT, &buff);
    printf("Updated permissions: %o\n", buff.msg_perm.mode & 0777);

    return 0;
}


