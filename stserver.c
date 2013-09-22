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

//Global var values
unsigned char debugflag = 0; //False by default

//Allocation and Free functions
stserver* allocstserv(void)
{
    stserver* s = malloc(sizeof(stserver));

    //Set each field to a default value
    s->sock = 0;
    s->port = NULL;
    s->address = NULL;
    s->confname = NULL;

    return s;
}

void freestserv(stserver* s)
{
    //Free struct fields
    free(s->port);
    free(s->address);
    free(s->confname);
    //Free struct
    free(s);
}


request* allocreq(void)
{
    request* r = malloc(sizeof(request));

    r->reqtype = NULL;
    r->reqfile = NULL;
    r->httpver = NULL;

    return r;
}

void freereq(request* r)
{
    //Free struct fields
    free(r->reqtype);
    free(r->reqfile);
    free(r->httpver);
    //Free struct
    free(r);
}

response* allocresp(void)
{
    response* r = malloc(sizeof(response));

    r->status =NULL;
    r->date =NULL;
    r->contype =NULL;
    r->contlenstr = NULL;
    r->contlen =0;

    return r;
}

void freeresp(response* r)
{
    //Free struct fields
    free(r->status);
    free(r->date);
    free(r->contype);
    free(r->contlenstr);
    //Free struct
    free(r);
}

configuration* allocconf(void)
{
    configuration* c = malloc(sizeof(configuration));   //Create struct
    
    //Set each value to a defualt value
    c->httpver = NULL;
    c->rootdir = NULL;
    c->extentions = NULL;
    c->nextentions = 0;

    return c;
}

void freeconf(configuration* c)
{
    //Free struct fiels
    free(c->httpver);
    free(c->rootdir);
    free(c->extentions);
    //Free struct
    free(c);
}

configuration* parseconf(char* confname)
{
    configuration* c = allocconf();
    
    FILE* f = fopen(confname, "r"); //Open file for reading

    if(f == NULL)
    {
        perror("fopen");
        exit(0);
    }

    char s [100];
    char* token;

    fgets(s,100,f);
    servdeblog("DirPath Line: %s", s);

    token = strtok(s," "); //Note, strtok is not threadsafe implementation
    servdeblog("Token: %s\n",token);

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
    servdeblog("Token: '%s'\n",token);

    int dirlen = strlen(token);

    servdeblog("Dirlen Token size: %d\n", dirlen);

    //Copy the dir name to a new char array of size n -2(brackets) -1(newline) 
    //+ 1 (null terminator). Copy dirlen -1(end bracket) -1(newline).
    //c->rootdir = strcpy(malloc((dirlen)*sizeof(char)),&token[1]);
    c->rootdir = strncpy(malloc((dirlen-2+1)*sizeof(char)),&token[1], dirlen-2);
    servdeblog("Home Directory: %s\n", c->rootdir);

    //Get filetypes
    fgets(s,100,f);
    servdeblog("File Line: %s", s);

    //c->extentions = malloc(); TODO finish this

    int len = strlen(s);

    if(fclose(f) !=0) //Close file and make sure it closes properly
    {
        perror("fclose");
        exit(0);
    }

    return c;
}

void exiterr(const char* format, ...)
{
    va_list args;
    va_start(args, format);
    vfprintf(stderr, format, args);
    va_end(args);
    exit(0);
}

void exitperr(const char* format, ...)
{
    //Print error and exit
    perror(format);
    exit(errno);
}

void servdeblog(const char* format, ...)
{
    if(!debugflag) return; //Return if we don't want to display debug info

    va_list args;
    va_start(args, format);
    vfprintf(stdout, format, args);
    va_end(args);
}
