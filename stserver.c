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

request* genreq(void)
{
    request* r = malloc(sizeof(request));

    r->reqtype = NULL;
    r->reqfile = NULL;
    r->httpver = NULL;

    return r;
}

response* genresp(void)
{
    response* r = malloc(sizeof(response));

    r->status =NULL;
    r->date =NULL;
    r->contype =NULL;
    r->contlenstr = NULL;
    r->contlen =0;

    return r;
}

configuration* genconf(void)
{
    configuration* c = malloc(sizeof(configuration));   //Create struct
    
    //Set each value to a defualt value
    c->httpver = NULL;
    c->rootdir = NULL;
    c->extentions = NULL;
    c->nextentions = 0;

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

    //Set home directory
    token = strtok(NULL," \r\n"); 
    printf("Token: '%s'\n",token);

    int dirlen = strlen(token);

    printf("Dirlen Token size: %d\n", dirlen);

    //Copy the dir name to a new char array of size n -2(brackets) -1(newline) 
    //+ 1 (null terminator). Copy dirlen -1(end bracket) -1(newline).
    //c->rootdir = strcpy(malloc((dirlen)*sizeof(char)),&token[1]);
    c->rootdir = strncpy(malloc((dirlen-2+1)*sizeof(char)),&token[1], dirlen-2);
    printf("Home Directory: %s\n", c->rootdir);

    //Get filetypes
    fgets(s,100,f);
    printf("File Line: %s", s);

    //c->extentions = malloc(); TODO finish this

    int len = strlen(s);

    if(fclose(f) !=0) //Close file and make sure it closes properly
    {
        perror("fclose");
        exit(0);
    }

    return c;
}
