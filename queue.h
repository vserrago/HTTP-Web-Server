//Constants
#define DEFAULTQVAL -1

//Struct definitions
typedef struct queue
{
    int  size;         //The size of the queue, in elements
    int  nextin;       //The element index to be filled in the next add
    int  nextout;      //The element to be retrieved in the next remove
    int* array;        //The storage for queue elements
} queue;

//Function prototypes
queue* createqueue(int size);
void freequeue(queue* q);
void qadd(queue* q, int value);
int qrem(queue* q);
int qpeek(queue* q);
int qhaselem(queue* q);
