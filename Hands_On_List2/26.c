/* Name: Kanan Gupta
 * Roll No. MT2024073
 * Problem: 
 * Write a program to send messages to the message queue. Check $ipcs -q
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
	printf("ENter the message type: \n");
	scanf("%ld", &mq.mtype);
	printf("Enter the message: \n");
	scanf(" %[^\n]",mq.message);
	size = strlen(mq.message);
	msgsnd(msgid,&mq,size+1,0);
}

