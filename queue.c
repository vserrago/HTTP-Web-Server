#include <stdlib.h>
#include <string.h>

//Local includes
#include "queue.h"
#include "stserver.h"

queue* createqueue(int size)
{
    queue* q = malloc(sizeof(queue));

    q->size = size;
    q->nextin = 0;
    q->nextout = 0;
    //Create queue and set default values
    q->array = malloc(size*sizeof(int));
    memset(q->array,DEFAULTQVAL,size*sizeof(int));
    return q;
}

void freequeue(queue* q)
{
    //Free array
    free(q->array);
    //Free struct
    free(q);
}

void qadd(queue* q, int value)
{
    //Check to see that queue is not full
    if(q->array[q->nextin] != DEFAULTQVAL)
        //TODO: Maybe return fail/success bool instead?
        exiterr("Error: Queue is full\n");

    //Add element
    q->array[q->nextin] = value;
    //Move to next element
    q->nextin++;
    q->nextin %= q->size;
}

int qrem(queue* q)
{
    int val;

    //Check to see that queue is not empty
    if(q->array[q->nextout] == DEFAULTQVAL)
        exiterr("Error: Removng value from empty queue\n");

    //Remove value from array
    val = q->array[q->nextout];
    q->array[q->nextout] = DEFAULTQVAL;

    //Move to next position
    q->nextout++;
    q->nextout %= q->size;
    return val;
}

/* Retrieves, but does not remove the next element out. If the queue is empty,
 * then the function returns DEFAULTQVAL
 */
int qpeek(queue* q)
{
    return q->array[q->nextout];
}

/* Returns true qpeek returns a non-default value
 */
int qhaselem(queue* q)
{
    //If qpeek is default return false, else true
    return (qpeek(q) == DEFAULTQVAL) ? 0 : 1;
}
