#include <stdio.h>
#include <stdlib.h>
#include <string.h>


//Copies a string into a newly allocated char array
char* cpynewstr(char* source) 
{
    int mlen = strlen(source)+1; 
    return strcpy(malloc(mlen*sizeof(char)), source); 
}
