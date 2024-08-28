/* Name: Kanan Gupta
 * Roll No. MT2024073
 * Problem: Write a program to create a zombie process.
 * */
 

#include<stdio.h>
#include<unistd.h>
#include<sys/wait.h>

int main(){

	if(!fork()){
		printf("Child PID: %d\n", getpid());
	}
	else getchar();
}


