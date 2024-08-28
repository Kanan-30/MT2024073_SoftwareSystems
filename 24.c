/* Name: Kanan Gupta
 * Roll No. MT2024073
 * Problem: Write a program to create an orphan process.
 * */
 

#include<stdio.h>
#include<unistd.h>
#include<sys/wait.h>

int main(){

	if(!fork()){

		printf("Child PPID before orphan: %d\n", getppid());
		sleep(3);
		printf("Child PPID after orphan: %d\n", getppid());
	}
}


