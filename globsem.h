#define SEM_MUTEX_KEY 0x00000004 /* Note: you should change this value        */ 
                                 /* to other unique values  such as your      */
                                 /* group# or etc. to not share the semaphore */
                                 /* with other groups. For example if your    */ 
                                 /* group number is 12, use 0x00000012        */
                                 /* Also after running your program by using   */
                                 /* ipcrm command remove the semaphores        */    


union semun 
{
        int val;                  /* value for SETVAL */
        struct semid_ds *buf;     /* buffer for IPC_STAT, IPC_SET */
        unsigned short *array;    /* array for GETALL, SETALL */
                                  /* Linux specific part: */
        struct seminfo *__buf;    /* buffer for IPC_INFO */
};

static int sem_mutex; // Semaphore id's
void setSemValue(int semaphore, int count);

int createSem();

void down(int semaphore);

void up(int semaphore);
