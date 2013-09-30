#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <signal.h>
#include <string.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>

//Local includes
#include "globsem.h"

void setSemValue(int semaphore, int count)
{
    union semun sem_union;

    sem_union.val = count;

    // Initalize semaphore with new count
    if ( semctl(semaphore, 0, SETVAL, sem_union) == -1)
    {
        fprintf(stderr, "(#%d) ", errno);
        perror("semctl() failed");
        exit(EXIT_FAILURE);
    }
    
}

int createSem()
{
    //sem_mutex = semget((key_t)SEM_MUTEX_KEY, 1, 0666 | IPC_CREAT);
    int sem_mutex = semget(IPC_PRIVATE,1,0666|IPC_CREAT);
     
    //printf("[**] Semaphore MUTEX successfully attached with id: %d\n", sem_mutex);

 
    // Initalize MUTEX semaphore
    setSemValue(sem_mutex, 1);
    //printf("[*] Semaphore MUTEX count initalized to 1.\n");
        
    return sem_mutex;
}
       
void down(int semaphore)
{
    struct sembuf semBuff;

    // Decrease semaphore count by 1
    semBuff.sem_num = 0;
    semBuff.sem_op = -1;
    semBuff.sem_flg = SEM_UNDO;

    if (semop(semaphore, &semBuff, 1) == -1)
    {
        fprintf(stderr, "(#%d) ", errno);
        perror("semop() failed");
        exit(EXIT_FAILURE);
    }
}

void up(int semaphore)
{
    struct sembuf semBuff;
    
    // Increase semaphore count by 1
    semBuff.sem_num = 0;
    semBuff.sem_op = +1;
    semBuff.sem_flg = SEM_UNDO;

    if (semop(semaphore, &semBuff, 1) == -1)
    {
        fprintf(stderr, "(#%d) ", errno);
        perror("semop() failed");
        exit(EXIT_FAILURE);
    }
}
