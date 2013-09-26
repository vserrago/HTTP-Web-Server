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
#include "util.h"

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

    r->badreq  = 0;     //Valid by default
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
        exitperr("fopen");
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
        exiterr("httpver error\n");
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
        exitperr("fclose");
    }

    return c;
}

void prepserv(stserver* serv)
{
    int status;
    struct addrinfo hints;
    struct addrinfo* servinfo;

    memset(&hints, 0, sizeof hints);    //Wipe space for hints
    hints.ai_family = AF_UNSPEC;        //Don't specify IP type
    hints.ai_socktype = SOCK_STREAM;    //Use TCP socket
    hints.ai_flags = AI_PASSIVE;        //Figure out ip

    //Get address info
    if ((status = getaddrinfo(serv->address, serv->port, &hints, &servinfo)) != 0)
    {
        exiterr("getaddrinfo error: %s\n", gai_strerror(status));
    }

    //Create socket
    serv->sock = socket(servinfo->ai_family, servinfo->ai_socktype, 
            servinfo->ai_protocol);
    if(serv->sock == -1)
    {
        exiterr("Socket error\n");
    }

    //Bind socket
    if(bind(serv->sock, servinfo->ai_addr, servinfo->ai_addrlen) != 0)
    {
        exiterr("binderror\n");
    }

    int backlog = 10; //Have a backlog of up to 10 requests

    //Listen on socket for incoming connections
    if(listen(serv->sock, backlog) != 0)
    {
        exiterr("Listen error\n");
    }
    freeaddrinfo(servinfo); //Free address info
}

char* recievereq(int sockfd)
{

    int  mbs = 1024;    //Maximum buffer size, size of 2KiB
    char buff [mbs];    //Buffer to recieve from
    int  br = 0;        //Bytes recieved

    char* reqstr = malloc(mbs*sizeof(char)); //Allocate space for string
    reqstr[0] = '\0';   //Make empty string 
       
    for(br=0; 0 < (br = recv(sockfd,buff, mbs,0)); )
    {
        //Concat contents into string
        strncat(reqstr,buff, br*sizeof(char));

        servdeblog("Request string: %s\n", reqstr);

        char* crlfs;
        if((crlfs = strstr(reqstr, "\r\n\r\n"))!= NULL)
        {
            servdeblog("End found\n");
            break;
        }
    }

    if(br < 0)
    {
        exiterr("Recieve Error\n");
    }

    if(br == 0)
    {
        exiterr("Connection Closed\n");
    }

    servdeblog("%d bytes recieved, Request:\n", strlen(reqstr));
    servdeblog("'%s'\n",reqstr);
    return reqstr;
}

request* parsereq(char* reqstr)
{
    request* r = allocreq();

    char* token;
    unsigned char badreqflag = 0; //Bad request

    //Parse header request
    if((token = strtok(reqstr," ")) == NULL)
    {
        servdeblog("Header not found\n");
        badreqflag = 1;
        goto badrequest;
    }
    /* Don't check for correct method type here, as that will be done when
     * creating a response so that a 501 NOT IMPLEMENTED can be returned.
     */
    r->reqtype = cpynewstr(token); 


    //Parse file request
    if((token = strtok(NULL," ")) == NULL)
    {
        servdeblog("File path not found\n");
        badreqflag = 1;
        goto badrequest;
    }

    //Check to see if a leading / is given in file path
    if(token[0] != '/')
    {
        servdeblog("Invalid filepath\n");
        badreqflag = 1;
        goto badrequest;
    }
        
    r->reqfile = cpynewstr(token); 


    //Parse HTTPver
    if((token = strtok(NULL," \r\n")) == NULL)
    {
        servdeblog("HTTPver not found\n");
        badreqflag = 1;
        goto badrequest;
    }

    //If httpver is both not 1.0 or 1.1
    if(strcmp(token,"HTTP/1.0") != 0 && strcmp(token, "HTTP/1.1") != 0)
    {
        servdeblog("Invalid HTTPver\n");
        badreqflag = 1;
        goto badrequest;
    }

    r->httpver = cpynewstr(token); 

    servdeblog("Reqtype: '%s', Reqfile: '%s', httpver: '%s'\n",  r->reqtype, r->reqfile, r->httpver);

badrequest:
    if(badreqflag)
    {
        /* If a request is found to be bad while parsing through it, then we
         * don't want to attempt to parse through the rest. If r->badreq is
         * true, then we don't care about the rest of request as it's invalid.
         */
        r->badreq = 1;
        servdeblog("Bad request detected\n");
    }

    return r;
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
    //TODO print out additional args
    //Print error and exit
    perror(format);
    exit(errno);
}

void servlog(const char* format, ...)
{
    va_list args;
    va_start(args, format);
    vfprintf(stdout, format, args);
    va_end(args);
}

void servdeblog(const char* format, ...)
{
    if(!debugflag) return; //Return if we don't want to display debug info

    va_list args;
    va_start(args, format);
    vfprintf(stdout, format, args);
    va_end(args);
}
