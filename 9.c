/* Name: Kanan Gupta
 * Roll No. MT2024073
 * Problem:  Write a program to print the following information about a given file.
a. inode
b. number of hard links
c. uid
d. gid
e. size
f. block size
g. number of blocks
h. time of last access
i. time of last modification
j. time of last change
 * */
 


#include<sys/stat.h>
#include<stdlib.h>
#include<stdio.h>
#include<time.h>

int main(int argc, char *argv[]){
	
	if(argc !=2){
		printf("Please enter filename");
	}
	
	struct stat filestat;

	if(stat(argv[1], &filestat)<0){

		perror("filestat not found!");

	}

	printf("Filename:%s\n", argv[1]);
	printf("a} Inode: %ld\n",(long) filestat.st_ino);
	printf("b} NUmber of hard links: %ld\n", (long)filestat.st_nlink);
	printf("c} Uid: %ld\n", (long)filestat.st_uid);
	printf("d} Gid: %ld\n", (long)filestat.st_gid);
	printf("e} Size: %ld\n", (long)filestat.st_size);
	printf("f} Block Size: %ld\n", (long)filestat.st_blksize);
	printf("g} NUmber of Blocks: %ld\n", (long)filestat.st_blocks);
	printf("h} Time of last access: %s\n", ctime(&filestat.st_atime));
	printf("i} Time of last modification: %s\n", ctime(&filestat.st_mtime));
	printf("j} TIme of Last Change: %s\n", ctime(&filestat.st_ctime));
	

}

