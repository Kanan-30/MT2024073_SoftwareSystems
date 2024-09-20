/* Name: Kanan Gupta
 * Roll No. MT2024073
 * Problem: Write a program to implement semaphore to protect any critical section.
a. rewrite the ticket number creation program using semaphore
b. protect shared memory from concurrent write access
c. protect multiple pseudo resources ( may be two) using counting semaphore
d. remove the created semaphore
 * */
 
#include <fcntl.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <semaphore.h>
#include <stdio.h>
#include <stdlib.h>

struct {
    int train_no;
    int ticket_count;
} db;

int main() {
    int fd, input;
    sem_t *sem;

    sem = sem_open("/ticket_booking_sem", O_CREAT, 0644, 1);
    if (sem == SEM_FAILED) {
        perror("sem_open");
        exit(EXIT_FAILURE);
    }

    fd = open("record.txt", O_RDWR);
    if (fd == -1) {
        perror("open");
        exit(EXIT_FAILURE);
    }

    printf("Select train number in which you want to book a ticket {1,2,3}\n");
    scanf("%d", &input);

    sem_wait(sem);

    lseek(fd, (input - 1) * sizeof(db), SEEK_SET);
    read(fd, &db, sizeof(db));

    printf("Outside the train booking page.....!!\n");

    printf("Ticket Number: %d\n", db.ticket_count);
    db.ticket_count++;

    lseek(fd, -1 * sizeof(db), SEEK_CUR);
    write(fd, &db, sizeof(db));

    printf("To book ticket, press Enter\n");
    getchar();
    getchar();

    sem_post(sem);

    printf("Ticket Booked!!.\n");

    close(fd);

    sem_close(sem);
    sem_unlink("/ticket_booking_sem");

    return 0;
}

