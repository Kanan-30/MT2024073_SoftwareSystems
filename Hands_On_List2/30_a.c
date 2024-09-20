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
#include <unistd.h>
int main() {
    
    key_t key = ftok(".", 'a');  
    int shmid = shmget(key, 1024, 0666 | IPC_CREAT);  

    
    char *data = shmat(shmid, 0, 0);  // Attach memory with read-write mode
 
    printf("Enter data to write in shared memory\n");
    scanf(" %[^\n]",data);
    getchar();
    return 0;
}


