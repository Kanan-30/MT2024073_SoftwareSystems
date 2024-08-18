/* Name: Kanan Gupta
 * Roll No. MT2024073
 * Problem:  Write a program to open an existing file with read write mode. Try O_EXCL flag also.
 * */

#include<fcntl.h>
#include<stdio.h>

int main(void){
        printf ("fd = %d\n", open("myfileQ4",O_CREAT|O_EXCL,0744));
	perror("fd :");
	// O_RDWR (Overwrites)
}


