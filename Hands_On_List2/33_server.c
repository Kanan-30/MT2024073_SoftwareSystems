/* Name: Kanan Gupta
 * Roll No. MT2024073
 * Problem:
 * Write a program to communicate between two machines using socket.
 * */

#include<stdio.h>
#include<unistd.h>
#include<sys/types.h>
#include<sys/socket.h>
#include<netinet/in.h>
#include<fcntl.h>
#include<sys/stat.h>

int main(){

struct sockaddr_in serv,cli;
int sd,sz,nsd;
char buff[100];
sd = socket(AF_UNIX, SOCK_STREAM, 0);
serv.sin_family = AF_UNIX;
serv.sin_addr.s_addr = INADDR_ANY;
serv.sin_port = htons(5055);  

bind(sd, (void *)(&serv), sizeof(serv));
listen(sd,5);
sz = sizeof(cli);
nsd = accept(sd, (void *)(&cli),&sz);
read(nsd,buff,sizeof(buff));
printf("Message from Client: %s\n",buff);
write(nsd,"Ack from Server\n",17);

}
