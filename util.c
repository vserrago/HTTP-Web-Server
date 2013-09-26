#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "util.h"

//Copies a string into a newly allocated char array
char* cpynewstr(char* source) 
{
    int mlen = strlen(source)+1; 
    return strcpy(malloc(mlen*sizeof(char)), source); 
}

//Combines two strings into a new string
char* cmbnewstr(char* str1, char* str2)
{
    //Get length of combined string
    int len = strlen(str1);
    len += strlen(str2);
    len += 1; //Add 1 for null-terminator

    //Allocate space for string
    char* cmbstr = malloc(len*sizeof(char));

    //Copy strings into new string
    strcpy(cmbstr, str1);
    strcat(cmbstr, str2);

    return cmbstr;
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
