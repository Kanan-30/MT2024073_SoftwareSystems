/* Name: Kanan Gupta
 * Roll No. MT2024073
 * Problem:
 * Write a program to create a concurrent server.
a. use fork
 * */
 
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <stdlib.h>

int main() {
    struct sockaddr_in serv, cli;
    int sd, nsd, sz;
    char buff[100];

    
    sd = socket(AF_UNIX, SOCK_STREAM, 0);

    serv.sin_family = AF_UNIX;
    serv.sin_addr.s_addr = INADDR_ANY;
    serv.sin_port = htons(5059);

    
    //bind(sd, (struct sockaddr *) &serv, sizeof(serv));
    if (bind(sd, (struct sockaddr *) &serv, sizeof(serv)) < 0) {
        perror("Bind failed");
        close(sd);
        exit(1);
    }

    listen(sd, 5);

    sz = sizeof(cli);

    while (1) {
        
        nsd = accept(sd, (struct sockaddr*) &cli,&sz);
        if (!fork()) {
            close(sd); 
            read(nsd, buff, sizeof(buff));
            printf("Message from Client: %s\n", buff);
            write(nsd, "Ack from Server\n", 17);
            close(nsd);
            exit(0); 
        } else {
            close(nsd);
        }
    }

    return 0;
}


