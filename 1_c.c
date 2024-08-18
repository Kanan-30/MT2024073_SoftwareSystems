#include<sys/types.h>
#include<fcntl.h>
#include<sys/types.h>
#include<sys/stat.h>
#include<unistd.h>

int main(){
	 mkfifo("pipe2",0744);
}
