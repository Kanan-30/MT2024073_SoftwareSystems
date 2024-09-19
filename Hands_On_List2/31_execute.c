/* Name: Kanan Gupta
 * Roll No. MT2024073
 * Problem: 
 Write a program to create a semaphore and initialize value to the semaphore.
a. create a binary semaphore
b. create a counting semaphore
 * */
 
#include<stdio.h>
#include<unistd.h>
#include<sys/sem.h>
#include<sys/types.h>
#include<sys/ipc.h>

int main(){
	int key,semid;

	key = ftok(".",'a');
	struct sembuf buff = {0,-1,0};
	semid = semget(key,1,0);
	printf("Entering the critical section..\n");
	printf("Waiting for unlock\n");
	semop(semid,&buff,1);
	printf("Inside the critical section...\n");
	printf("Press Enter to UNlock\n");
	getchar();
	buff.sem_op=1;
	semop(semid,&buff,1);
}

