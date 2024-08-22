/* Name: Kanan Gupta
 * Roll No. MT2024073
 * Problem: Write a program to find out the opening mode of a file. Use fcntl
 * */
 

#include<unistd.h>
#include<fcntl.h>
#include<stdlib.h>
#include<stdio.h>


//F_GETFL (void)
  //            Return (as the function result) the file access mode and the file status flags; arg is ignored.
//
int main(){

	int fd1,fd2,fd3;

	fd1 = open("Q12_FIle1.txt",O_RDONLY);
	fd2 = open("Q12_FIle1.txt",O_WRONLY);
	fd3 = open("Q12_FIle1.txt",O_RDWR);
	
	if(fd1==-1){
		perror("file doesnot exist");
	}
	
	int flag1 = fcntl(fd1,F_GETFL);
	int flag2 = fcntl(fd2,F_GETFL);
	int flag3 = fcntl(fd3,F_GETFL);
	
		
	int access_mode1 = flag1 &  O_ACCMODE;
	printf("File Access Mode for fd1: %d\n",access_mode1);

	int access_mode2 = flag2 & O_ACCMODE;
	printf("File Access Mode for fd2: %d\n", access_mode2);

	int access_mode3 = flag3 & O_ACCMODE;
	printf("File Access Mode for fd3: %d\n", access_mode3);
	
	//if(access_mode1 == 0) printf("File Access Mode:Read Only\n");
	
	//if(access_mode2 == 0) printf("File Access Mode: Write Only\n ");  

	//if(access_mode3 == 0) printf("File Access Mode:  Read Write \n");  

	close(fd1);
	close(fd2);
	close(fd3);

}

