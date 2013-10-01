//Struct definitions
typedef struct queue
{
    int  qsize;         //The size of the queue, in elements
    int  curindex;      //The element next up in the queue
    int* qarray;        //The storage for queue elements
}; queue

//Function prototypes
queue* createqueue(int size);
void freequeue(queue* q);
void queueadd(int element);
int queuerem(void);
