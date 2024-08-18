#include<fcntl.h>
#include<stdio.h>

int main(void){
        printf ("fd = %d\n", open("myfileQ4",O_CREAT|O_EXCL,0744));
	perror("fd :");
	// O_RDWR (Overwrites)
}


