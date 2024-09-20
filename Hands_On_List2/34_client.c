/* Name: Kanan Gupta
 * Roll No. MT2024073
 * Problem: 
 * Write a program to create a concurrent server.
a. use fork
b. use pthread_create

 * */
 
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>

int main() {
    struct sockaddr_in serv;
    int sd;
    char buff[100];
    sd = socket(AF_UNIX, SOCK_STREAM, 0);

    serv.sin_family = AF_UNIX;
    serv.sin_addr.s_addr = INADDR_ANY;
    serv.sin_port = htons(5061);

   
    connect(sd ,(void *) (&serv),sizeof(serv));

    write(sd, "Hello Server, I am your client\n", 30);
    read(sd, buff, sizeof(buff));
    printf("Message from Server: %s\n", buff);

    close(sd);
    return 0;
}


