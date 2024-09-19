/* Name: Kanan Gupta
 * Roll No. MT2024073
 * Problem: 
 * Write a program to create a semaphore and initialize value to the semaphore.
a. create a binary semaphore
b. create a counting semaphore
 * */
 
#include<unistd.h>
#include<stdio.h>
#include<sys/sem.h>
#include<sys/types.h>
#include<sys/ipc.h>

union semun{
	int val;
	struct semid_ds *buff;
	unsigned short int *Arr;
};

int main(){
	union semun arg;
	int key,semid;

	key = ftok(".",'a');
	semid = semget(key,1,IPC_CREAT|0644);
	arg.val = 3;
	semctl(semid,0,SETVAL,arg);
}

