#include <stdio.h>
#include <stdlib.h>
#include <string.h>


//Copies a string into a newly allocated char array
char* cpynewstr(char* source) 
{
    int mlen = strlen(source)+1; 
    return strcpy(malloc(mlen*sizeof(char)), source); 
}

int filesize(FILE* f)
{
    int n; //Number of bytes

    //Count number of bytes
    for(n=0; getc(f)!=EOF; n++)
        ;
    rewind(f); //Return file to beginning

    return n; 
}
