/* Name: Kanan Gupta
 * Roll No. MT2024073
 * Problem:
 * 30. Write a program to create a shared memory.
a. write some data to the shared memory
b. attach with O_RDONLY and check whether you are able to overwrite.
c. detach the shared memory
d. remove the shared memory
 * */
 
#include <stdio.h>
#include <stdlib.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <string.h>
int main() {
    
    key_t key = ftok(".", 'a');  
    int shmid = shmget(key, 1024, 0666);
    char *data = shmat(shmid,0, 0);
    shmdt(data);
    printf("Detached from shared memory.\n");

    return 0;
}


