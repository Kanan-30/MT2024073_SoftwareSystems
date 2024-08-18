/* Name: Kanan Gupta
 * Roll No. MT2024073
 * Problem:  Write a program to copy file1 into file2 ($cp file1 file2).
 * */
 

#include<unistd.h>
#include<fcntl.h>
#include<stdlib.h>
#include<stdio.h>


int main(int argc, char *argv[]){

	int src_fd, dest_fd; // source and destination file descriptors
	ssize_t read_bytes, write_bytes; // size of bytes read or to write
	char buff[1024]; // to stores bytes read in buffer

	if(argc!=3){ //  3 command line arguments argv[0] -> executable file, argv[1]-> source file, argv[2] -> destination file
		printf("Please enter the source file and destination file names");
		exit(EXIT_FAILURE);
	}

	src_fd = open(argv[1],O_RDONLY);
	dest_fd = open(argv[2], O_WRONLY | O_CREAT ,  0644);


	while ((read_bytes= read(src_fd, buff, sizeof(buff))) > 0) {
        write_bytes = write(dest_fd, buff, read_bytes);
       	}

	 close(src_fd);
         close(dest_fd);




}

