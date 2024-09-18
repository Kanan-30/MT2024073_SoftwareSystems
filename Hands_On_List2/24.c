/* Name: Kanan Gupta
 * Roll No. MT2024073
 * Problem: Write a program to create a message queue and print the key and message queue id.
 * */
 
#include<unistd.h>
#include<stdio.h>
#include<sys/msg.h>
#include<sys/ipc.h>
#include<sys/types.h>

int main(){
	int msgid;
	key_t key;

	key = ftok("." , 'a');
	msgid = msgget(key,IPC_CREAT|IPC_EXCL|0744);
	printf("key value = 0x%0x\n" , key);
	printf("Message Queue id = %d\n",msgid);
}
