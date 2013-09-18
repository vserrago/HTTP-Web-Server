#include <arpa/inet.h>
#include <errno.h>
#include <limits.h>
#include <netdb.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/errno.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

//Local includes
#include "stserver.h"

configuration* genconf(void)
{
    configuration* c = malloc(sizeof(configuration));   //Create struct
    
    //Set each value to a defualt value
    c->httpver = NULL;
    c->rootdir = NULL;
    c->extentions = NULL;

    return c;
}

configuration* parseconf(char* confname)
{
    configuration* c = genconf();
    
    FILE* f = fopen(confname, "r"); //Open file for reading

    if(f == NULL)
    {
        perror("fopen");
        exit(0);
    }

    char s [100];
    char* token;

    fgets(s,100,f);
    printf("DirPath Line: %s", s);

    token = strtok(s," "); //Note, strtok is not threadsafe implementation
    printf("Token: %s\n",token);

    //Determine http version
    if(strcmp("HTTP1.0",token) == 0)
        c->httpver = "1.0";
    else
    {
        printf("httpver error\n");
        exit(0);
    }

    fgets(s,100,f);
    printf("File Line: %s", s);

    if(fclose(f) !=0) //Close file and make sure it closes properly
    {
        perror("fclose");
        exit(0);
    }

    return c;
}
