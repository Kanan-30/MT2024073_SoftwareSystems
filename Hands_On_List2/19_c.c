/* Name: Kanan Gupta
 * Roll No. MT2024073
 * Problem:
 *
 * Q19. Create a FIFO file by
a. mknod command
b. mkfifo command
c. use strace command to find out, which command (mknod or mkfifo) is better.
c. mknod system call
d. mkfifo library function
 * */


#include<sys/types.h>
#include<fcntl.h>
#include<sys/types.h>
#include<sys/stat.h>
#include<unistd.h>
#include<fcntl.h>

int main(){
	 mknod("pipe1", S_IFIFO | 0666, 0);
}
