/* Name: Kanan Gupta
 * Roll No. MT2024073
 * Problem:  Write a program to take input from STDIN and display on STDOUT. Use only read/write system calls
 * */
 
#include<unistd.h>

int main(){

	char buffer[1024];
	ssize_t bytes_read; //signed data type -> to return size -> to store no of bytes read and to write

	while((bytes_read = read(STDIN_FILENO, buffer, sizeof(buffer)))>0){ // read upto size of buffer -> STDIN (fd =0), store in buffer -> return 0 when EOF
		write(STDOUT_FILENO, buffer, bytes_read);    // writes from buffer to stdout
	}

	return 0;
}

