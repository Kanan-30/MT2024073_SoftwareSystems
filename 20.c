/* Name: Kanan Gupta
 * Roll No. MT2024073
 * Problem: Find out the priority of your running program. Modify the priority with nice command.
 * */
 

#include <stdio.h>
#include <unistd.h>
#include <sys/resource.h>
#include <sys/time.h>

int main() {
    int program_priority;

    
    program_priority = getpriority(PRIO_PROCESS, 0);
    printf("Current Priority: %d\n", program_priority);

    nice(10);

    program_priority = getpriority(PRIO_PROCESS, 0);
    printf("New Priority: %d\n", program_priority);

    return 0;
}


