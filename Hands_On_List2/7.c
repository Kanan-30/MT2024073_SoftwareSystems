/* Name: Kanan Gupta
 * Roll No. MT2024073
 * Problem: 
 * Write a simple program to print the created thread ids.
 */


#include <stdio.h>
#include <pthread.h>

void* Q7_thread_function(void* arg) {
    printf("Thread Created. Thread ID: %lu\n", pthread_self());
    return NULL;
}

int main() {
    pthread_t thread1, thread2, thread3;

    pthread_create(&thread1, NULL, Q7_thread_function, NULL);
    pthread_create(&thread2, NULL, Q7_thread_function, NULL);
    pthread_create(&thread3, NULL, Q7_thread_function, NULL);

    pthread_join(thread1, NULL);
    pthread_join(thread2, NULL);
    pthread_join(thread3, NULL);

    return 0;
}



