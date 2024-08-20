/* Name: Kanan Gupta
 * Roll No. MT2024073
 * Problem:  Write a program to open a file in read only mode, read line by line and display each line as it is read.
Close the file when end of file is reached.
 * */
 

#include<unistd.h>
#include<fcntl.h>
#include<stdlib.h>
#include<stdio.h>


int main(int argc, char *argv[]){

	int fd;
	ssize_t read_bytes; // size of bytes read or to write
	char buff[1024]; // to stores bytes read in buffer

	fd = open("Q8_Input.txt",O_RDONLY);

	int i=0;

	while ((read_bytes= read(fd, &buff[i],1)) > 0) {
        	if(buff[i]=='\n' || i == sizeof(buff)-1){

			buff[i+1] ='\0';
			printf("%s", buff);

			//printf("Press Enter to read next line\n");
			//getchar();
			i=0;
		}
		else{
			i++;
		}
       	}

	 close(fd);
}

