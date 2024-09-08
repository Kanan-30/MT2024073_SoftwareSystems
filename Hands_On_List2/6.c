/* Name: Kanan Gupta
 * Roll No. MT2024073
 * Problem: 
 Write a simple program to create three threads.
 * */
 
#include <stdio.h>
#include <pthread.h>

void* Q6_thread_function(void* arg) {
    printf("Thread Created\n");
    return NULL;
}

void main() {
    pthread_t tid;
    pthread_create(&tid,NULL,Q6_thread_function,NULL);
    pthread_create(&tid,NULL,Q6_thread_function,NULL);
    pthread_create(&tid,NULL,Q6_thread_function,NULL);
    pthread_join(tid,NULL);
}


