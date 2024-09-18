/* Name: Kanan Gupta
 * Roll No. MT2024073
 * Problem:
 * Write a program to receive messages from the message queue.
b. with IPC_NOWAIT as a flag 
 * */
 
#include<unistd.h>
#include<stdio.h>
#include<sys/msg.h>
#include<sys/ipc.h>
#include<sys/types.h>
#include<string.h>

int main(){

        int msgid,size;
        key_t key;
        struct msg{
                long int mtype;
                char message[100];
        }mq;

        key = ftok(".",'a');
        msgid = msgget(key,0);
        printf("ENter the message type to receive: \n");
        scanf("%ld", &mq.mtype);
      
	msgrcv(msgid,&mq,sizeof(mq.message),mq.mtype,IPC_NOWAIT);
	printf("Message type: %ld\n",mq.mtype);
	printf("Message text: %s\n",mq.message);
}



