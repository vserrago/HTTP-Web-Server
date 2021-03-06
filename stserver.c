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
#include <time.h>
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
    r->content = NULL;
    r->contlen = 0;

    return r;
}

void freereq(request* r)
{
    //Free struct fields
    free(r->reqtype);
    free(r->reqfile);
    free(r->httpver);
    free(r->content);
    //Free struct
    free(r);
}

response* allocresp(void)
{
    response* r = malloc(sizeof(response));

    r->status       = NULL;
    r->date         = NULL;
    r->contype      = NULL;
    r->contlenstr   = NULL;
    r->content      = NULL;
    r->contlen      = 0;

    return r;
}

void freeresp(response* r)
{
    //Free struct fields
    free(r->status);
    free(r->date);
    free(r->contype);
    free(r->contlenstr);
    free(r->content);
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
    //TODO do more error checking
    configuration* c = allocconf();

    FILE* f = fopen(confname, "r"); //Open file for reading

    if(f == NULL)
        exitperr("fopen");

    int readsize = 100;
    char s [readsize];
    char* token;

    fgets(s,readsize,f);
    servdeblog("DirPath Line: %s", s);

    token = strtok(s," "); //Note, strtok is not threadsafe implementation
    servdeblog("Token: %s\n",token);

    //Determine http version
    if(strcmp("HTTP1.0",token) == 0)
        c->httpver = cpynewstr("HTTP/1.0");
    else if(strcmp("HTTP1.1",token) == 0)
        //Though we can serve 1.1 requests, we only serve as an HTTP1.0 server
        exiterr("Server can not be HTTP1.1\n"); 
    else
        exiterr("Http version configuration error \n");

    //Set home directory
    token = strtok(NULL," \r\n"); 
    servdeblog("Token: '%s'\n",token);

    int dirlen = strlen(token);

    servdeblog("Dirlen Token size: %d\n", dirlen);

    /* Copy the dir name to a new char array of size n -2(brackets) + 1 (null-
     * terminator). Copy dirlen -1(begin bracket) -1(end bracket). Copy from
     * the first element +1, to avoid the begin bracket.
     */
    c->rootdir = strncpy(malloc((dirlen-2+1)*sizeof(char)),&token[1], dirlen-2);
    servdeblog("Home Directory: %s\n", c->rootdir);

    //Get filetypes
    fgets(s,readsize,f);
    servdeblog("File Line: %s", s);

    c->extentions = cpynewstr(s);

    //Read pool line
    fgets(s,readsize,f);
    servdeblog("File Line: %s", s);

    //POOL identifier
    token = strtok(s," \r\n"); 
    servdeblog("Token: '%s'\n",token);

    //Ensure that identifier is correct
    if(strcmp("POOL",token) != 0)
        exiterr("%s is missing POOL identifier", confname);

    //Pool number
    token = strtok(NULL," \r\n"); 
    c->poolsize = strtol(token, NULL, 10);
    servdeblog("Token: '%s', Pool integer value: '%d'\n", token, c->poolsize);


    //Read queue line
    fgets(s,readsize,f);
    servdeblog("File Line: %s", s);

    //QUEUE identifier
    token = strtok(s," \r\n"); 
    servdeblog("Token: '%s'\n",token);

    //Ensure that identifier is correct
    if(strcmp("QUEUE",token) != 0)
        exiterr("%s is missing QUEUE identifier", confname);

    //Queue number
    token = strtok(NULL," \r\n"); 
    c->queuesize = strtol(token, NULL, 10);
    servdeblog("Token: '%s', Queue integer value: '%d'\n", token, c->queuesize);


    if(fclose(f) !=0) //Close file and make sure it closes properly
        exitperr("fclose");

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

    int yes=1;

    //Make socket reusable
    if (setsockopt(serv->sock,SOL_SOCKET,SO_REUSEADDR,&yes,sizeof(int)) == -1) 
    {
        exitperr("setsockopt");
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

/* Recieves from a connection specified by sockfd and then returns a string
 * containing the entire request. Returns NULL if connection was closed by the
 * client while recieving from it.
 */
char* recievereq(int sockfd)
{
    int  mbs = 1024;    //Maximum buffer size, size of 2KiB
    char buff [mbs];    //Buffer to recieve from
    int  br = 0;        //Bytes recieved
    unsigned char postflag = 0; //If we are waiting for a post request

    //Alloc space for request string
    char* reqstr = newemptystr(mbs*sizeof(char));

    for(br=0; 0 < (br = recv(sockfd,buff, mbs,0)); )
    {
        //Concat contents into string
        strncat(reqstr,buff, br*sizeof(char));

        servdeblog("Request string: %s\n", reqstr);

        char* contlen;
        char* crlfs;
        //Check if there are two CRLFs if the postflag is not true
        if(!postflag && (crlfs = strstr(reqstr, "\r\n\r\n"))!= NULL)
        {
            //Check for content length header //16 chars until number
            if((contlen = strstr(reqstr, "Content-Length:"))!= NULL)
            {
                postflag = 1;
                crlfs += 4; //Move past the two CRLFs
                servdeblog("Postflag\n");
            }
            else
            {
                servdeblog("Regular End found\n");
                break;
            }
        }
        else if(postflag && (crlfs = strstr(reqstr, "\r\n"))!= NULL)
        {
            servdeblog("POST End found\n");
            break;
        }
    }

    //If loop ends/breaks with a br of less than 1, then an error occured
    if(br < 0)
        exiterr("Recieve Error\n");
    if(br == 0)
    {
        free(reqstr); //Free unused request
        return NULL;
    }

    servdeblog("%d bytes recieved, Request:\n", strlen(reqstr));
    servdeblog("'%s'\n",reqstr);
    return reqstr;
}

request* parsereq(char* reqstr)
{
    /* Save copy of request string so that it can be parsed properly, since
     * strtok modifies the string as it tokenizes it.
     */
    char* postreqstr = cpynewstr(reqstr);

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

    int namelen = strlen(token);

    //If the token ends in a /, concatenate the default page onto it
    if(token[namelen-1] == '/')
        r->reqfile = cmbnewstr(token, DEFAULTFILENAME);
    else
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

    char* contlenp;
    //HTTP POST-specific parsing
    if(strcmp(r->reqtype, "POST") == 0)
    {
        //If content-length exists
        if((contlenp = strstr(postreqstr,"Content-Length")) != NULL)
        {
            r->contlen = atoi(contlenp+(16*sizeof(char)));
            servdeblog("Content Length: %d\n",r->contlen);
        }
        else
        {
            servdeblog("No contlen\n");
            badreqflag = 1;
            goto badrequest;
        }

        char* crlfs;
        if((crlfs = strstr(postreqstr, "\r\n\r\n"))!= NULL)
        {
            crlfs +=4; //go to beginning of chars to read in
            r->content = newemptystr((r->contlen+1)*sizeof(char));
            strncat(r->content,crlfs,r->contlen);
            servdeblog("Read-in content: '%s'\n",r->content);
        }
        else
        {
            servdeblog("No actual content in '%s'\n", postreqstr);
            badreqflag = 1;
            goto badrequest;
        }

    }

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

    free(postreqstr); //Free string

    return r;
}

response* handlereq(request* req, configuration* config)
{
    unsigned char notimplflag = 0; //Not implemented flag
    unsigned char badpermflag = 0; //No read permissions flag
    unsigned char notfndflag  = 0; //Not found flag
    unsigned char headreqflag = 0; //Error must return head if requesting head

    char* filepath = NULL; //Full path for requested file
    FILE* f;

    response* resp = allocresp();
    //If invalid request
    if(req->badreq)
    {
        goto errors;
    }

    //If get or head request
    if(strcmp(req->reqtype, "GET") == 0 || strcmp(req->reqtype, "HEAD") == 0)
    {
        //Set head flag in case of an error
        if(strcmp(req->reqtype, "HEAD") == 0)
            headreqflag = 1;

        filepath = cmbnewstr(config->rootdir, req->reqfile);
        servdeblog("File path: %s\n", filepath);

        //Open file
        if((f = fopen(filepath,"r")) == NULL)
        {
            //If file does not exist
            if(errno == ENOENT)
            {
                servdeblog("File not found\n");
                notfndflag = 1;
            }
            //Else bad read permissions
            else //TODO find errno code
            {
                servdeblog("File cannot be read\n");
                badpermflag = 1;
            }

            //Either way go to errors
            goto errors;
        }

        //Set response info
        resp->status = cpynewstr(ST200); //Request is ok
        resp->date = getdate();
        resp->contype= cpynewstr(CONTYPETEXT);
        //Allocate enough space for any content size
        resp->contlenstr= malloc(50*sizeof(char)); 

        //Get file length
        resp->contlen = filesize(f);
        //Print length to string
        sprintf(resp->contlenstr, "Content-Length %d", resp->contlen);

        //Only add content if reqtype is GET
        if(strcmp(req->reqtype, "GET") == 0)
        {
            //Add content to response
            resp->content = readfile(resp->contlen, f);
            servdeblog("Content to send: '%s'\n",resp->content);
        }
        fclose(f);
    }
    //If post request
    else if(strcmp(req->reqtype, "POST") == 0)
    {
        //Attempt to open file
        filepath = cmbnewstr(config->rootdir, req->reqfile);
        servdeblog("File path: %s\n", filepath);

        if((f = fopen(filepath,"w")) == NULL)
        {
            servdeblog("Post open file error\n");
            //Only thing that can happen is bad permissions
            badpermflag = 1;
            goto errors;
        }

        //Write data to file
        if( fputs(req->content, f) == EOF)
        {
            servdeblog("Fputs EOF error\n");
            badpermflag = 1;
            goto errors;
        }
        fclose(f);

        resp->status = cpynewstr(ST201);
        resp->date = getdate();

        if((f = fopen(HTML201,"r")) == NULL)
            exiterr("Error file error\n");
        //Get content length
        resp->contlen = filesize(f);
        resp->contlenstr= malloc(50*sizeof(char)); 
        sprintf(resp->contlenstr, "Content-Length %d", resp->contlen);

        //If get req
        if(!headreqflag)
            resp->content = readfile(resp->contlen, f);
        fclose(f);
    }
    //Else unimplemented request
    else
    {
        servdeblog("Header not implemented: %s\n", req->reqtype);
        notimplflag = 1;
        goto errors;
    }

errors: //Handle errors
    //If at least one error has occurred
    if(req->badreq || notimplflag || badpermflag || notfndflag)
    {
        //Get rid of previous info, and fill out resp with new info
        freeresp(resp);
        resp = allocresp();

        resp->date = getdate();
        resp->contype = cpynewstr(CONTYPETEXT);

        if(req->badreq)
        {
            resp->status = cpynewstr(ST400);

            if((f = fopen(HTML400,"r")) == NULL)
                exiterr("Error file error\n");
        }
        else if(notimplflag)
        {
            resp->status = cpynewstr(ST501);

            if((f = fopen(HTML501,"r")) == NULL)
                exiterr("Error file error\n");
        }
        else if(badpermflag)
        {
            resp->status = cpynewstr(ST403);

            if((f = fopen(HTML403,"r")) == NULL)
                exiterr("Error file error\n");
        }
        else if(notfndflag)
        {
            resp->status = cpynewstr(ST404);

            if((f = fopen(HTML404,"r")) == NULL)
                exiterr("Error file error\n");
        }

        //Get content length
        resp->contlen = filesize(f);
        resp->contlenstr= malloc(50*sizeof(char)); 
        sprintf(resp->contlenstr, "Content-Length %d", resp->contlen);

        //If get req
        if(!headreqflag)
            resp->content = readfile(resp->contlen, f);
        fclose(f);
    }

    //Cleanup
    if(filepath != NULL)
        free(filepath);

    return resp;
}

//Gets the current date
char* getdate(void)
{
    time_t t; //Time variable
    time(&t); //Get system time
    //Return system time as a string
    return cmbnewstr("Date: ", ctime(&t));
}

void sendresp(int sockfd, response* resp)
{
    int resplen = 0; //Response length in bytes
    //Get lengths of each response section, if it isn't null
    if(resp->status != NULL)
        resplen += strlen(resp->status);
    if(resp->date != NULL)
        resplen += strlen(resp->date);
    if(resp->contype != NULL)
        resplen += strlen(resp->contype);
    if(resp->contlenstr != NULL)
        resplen += strlen(resp->contlenstr);
    if(resp->content != NULL)
        resplen += strlen(resp->content);

    //resplen += 4; //Add two CRLFs
    resplen += 6; //Add three CRLFs
    resplen += 1; //Add 1 for null-terminator
    //resplen += 10; //Add extra space

    servdeblog("Response is %d bytes long\n", resplen);

    //Make string
    char* respstr = newemptystr(resplen*sizeof(char));

    //Turn response into 1 long string
    if(resp->status != NULL)
        strcat(respstr, resp->status);
    if(resp->date != NULL)
        strcat(respstr, resp->date);
    if(resp->contype != NULL)
        strcat(respstr, resp->contype);
    if(resp->contlenstr != NULL)
        strcat(respstr, resp->contlenstr);
    //Add two CRLFs
    strcat(respstr, "\r\n\r\n");
    if(resp->content != NULL)
    {
        strcat(respstr, resp->content);
        strcat(respstr, "\r\n");
    }

    servdeblog("String to send: '%s'\n",respstr);

    int bs;  //Bytes sent
    int tbs; //Total bytes sent

    for(bs=0, tbs=0; (bs = send(sockfd, respstr+(tbs*sizeof(char)),resplen-tbs-1,0)) != 0; tbs += bs)
    {
        servdeblog("Bytes sent: %d\n", bs);
        if(bs < 0)
            exitperr("send");
    }

    servlog("Request served. %d bytes sent.\n",tbs);

    //Free string
    free(respstr);
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
