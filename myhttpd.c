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


//Global Vars
int sock;                           //Socket file descriptor
char* port = "61000";               //TODO un-hardcode this!
char* address = "localhost";        //TODO unhardcode this too!
//unsigned char debugflag =0;         //Whether debug logging is enabled or not
char* confname = "myhttpd.conf";    //Name of config file

configuration* config;              //Config struct var

//Copies a string into a newly allocated char array
char* cpynewstr(char* source) 
{
    int mlen = strlen(source)+1; 
    return strcpy(malloc(mlen*sizeof(char)), source); 
}

int main(int argc, char *argv [])
{
    int i, j;  //Loop Variables
    unsigned char portflag; //Command line param flags
    struct sockaddr_storage socket_st;
    int status;
    struct addrinfo hints;
    struct addrinfo* servinfo;

    /* Arg Parsing */
    for(i=1; i<argc; i++)
    {
        servdeblog("Param: %s\n", argv[i]);
        if(argv[i][0] == '-')
        {
            int len = strlen(argv[i]);
            for(j=1; j<len; j++)
            {
                switch(argv[i][j])
                {
                    case 'p': 
                        servdeblog("p param given\n");
                        portflag = 1;
                        break;
                    case 'd':
                        servdeblog("d param given\n");
                        debugflag = 1;
                        break;
                    default:
                        exiterr("Unkown param given\n");
                        break;
                }
            }
        }
        else if(portflag)
        {
            port = argv[i];
            int portnum = atoi(port);
            //Ensure that portnumber is in the range of useable ports
            if(portnum < MINPORTNUM || USHRT_MAX < portnum)
            {
                exiterr("Portnumber is invalid\n");
            }
            servdeblog("Portnum is %s\n", port);
        }
        else
        {
            exiterr("Unkown arg given\n");
        }
    }

    config = parseconf(confname);   //Parse configuration file and get info from it.


    /* Socket Sutff */
    memset(&hints, 0, sizeof hints);    //Wipe space for hints
    hints.ai_family = AF_UNSPEC;        //Don't specify IP type
    hints.ai_socktype = SOCK_STREAM;    //Use TCP socket
    hints.ai_flags = AI_PASSIVE;        //Figure out ip


    //Get address info
    if ((status = getaddrinfo(address, port, &hints, &servinfo)) != 0)
    {
        exiterr("getaddrinfo error: %s\n", gai_strerror(status));
    }

    //Create socket
    sock = socket(servinfo->ai_family, servinfo->ai_socktype, servinfo->ai_protocol);
    if(sock == -1)
    {
        exiterr("Socket error\n");
    }

    //Bind socket
    if(bind(sock, servinfo->ai_addr, servinfo->ai_addrlen) != 0)
    {
        exiterr("binderror\n");
    }

    int backlog = 10; //Have a backlog of up to 10 requests

    //Listen on socket for incoming connections
    if(listen(sock, backlog) != 0)
    {
        exiterr("Listen error\n");
    }

    socklen_t addr_size = sizeof socket_st;

    servlog("Ready to recieve\n");
    for(;;) //Forever
    {
        int c = accept(sock, (struct sockaddr *) &socket_st, &addr_size);

        if(c < 0)
        {
            exiterr("Accept error\n");
        }

        for(;;)
        {
            char  buff [1000];
            int bytesrecieved = recv(c,buff, 1000,0);

            if(bytesrecieved < 0)
            {
                exiterr("Recieve Error\n");
            }

            if(bytesrecieved == 0)
            {
                exiterr("Connection Closed\n");
            }

            servdeblog("%d bytes recieved, Request:\n", bytesrecieved);
            servdeblog("%s\n",buff);

            request* r = allocreq();
            char* token;

            int n;

            //Parse header request
            if((token = strtok(buff," ")) == NULL)
            {
                exiterr("Buff Token error\n");
            }
            r->reqtype = cpynewstr(token); 

            if((token = strtok(NULL," ")) == NULL)
            {
                exiterr("Buff Token error\n");
            }
            r->reqfile = cpynewstr(token); 

            if((token = strtok(NULL," \r\n")) == NULL)
            {
                exiterr("Buff Token error\n");
            }
            r->httpver = cpynewstr(token); 

            servdeblog("Reqtype: '%s', Reqfile: '%s', httpver: '%s'\n",  r->reqtype, r->reqfile, r->httpver);


            //Look for header ending 
            for(i=1;i<1000;i++)
            {
                if(buff[i] == '\r' && buff[i-1] == '\n')
                {
                    servdeblog("CLRF WAS found\n");
                    break;
                }
            }
            //Check to see if loop parsed through entire buffer
            if(i == 1000-1)
            {
                exiterr("CLRF not found\n");
            }

            /*
               servdeblog("Message Recieved: %s", buff);
               if (send(c, "Hello, world!", 13, 0) == -1)
               exitperr("send");
               //*/

            //n = (strlen(config->rootdir) + strlen(r->reqfile) + 1)
            n = strlen(config->rootdir);
            n += strlen(r->reqfile) + 2;
            //Combine paths
            char* abspath = malloc(
                    n*sizeof(char));
            strcpy(abspath, config->rootdir);
            strcat(abspath, r->reqfile);

            servdeblog("Abspath: '%s'\n", abspath);

            FILE* f = fopen(abspath, "r"); //Open file for reading
            if(f == NULL)
            {
                exitperr("fopen");
            }

            response* resp = allocresp();
            resp->status = cpynewstr("HTTP/1.0 200 OK\r\n");
            resp->date = cpynewstr("Date: Fri, 31 Dec 1999 23:59:59 GMT\r\n");
            resp->contype= cpynewstr("Content-Type: text/html\r\n");
            resp->contlenstr= malloc(50*sizeof(char));

            for(n=0; getc(f)!=EOF; n++);
            resp->contlen = n;
           // rewind(f);
            if(fseek(f, 0L, SEEK_SET) == -1)
            {
                exitperr("fseek");
            }

            sprintf(resp->contlenstr, "Content-Length %d", resp->contlen);

            servdeblog("Content Length: %d\n", resp->contlen);
            servdeblog("Content Length String: %s\n", resp->contlenstr);

            //Get length of response total
            n = strlen(resp->status) + strlen(resp->date) + strlen(resp->contype) + strlen(resp->contlenstr) + 10 + resp->contlen;

            //Make response buffer
            char* respbuff = malloc(n*sizeof(char));
            int buffsize = n;
            
            strcpy(respbuff, resp->status);
            strcat(respbuff, resp->date);
            strcat(respbuff, resp->contype);
            strcat(respbuff, resp->contlenstr);
            strcat(respbuff, "\r\n");
            strcat(respbuff, "\r\n");


            n = strrchr(respbuff, '\0') - respbuff;

            servdeblog("Header: '%s'\n", respbuff);
            servdeblog("Last Occurrance of null char: %d\n", n);

            //Copy file into buffer
            for(i=n;(respbuff[i] = getc(f))!=EOF; i++);


            servdeblog("Buffer: '%s\n'", respbuff);

            //for(n=0;(n += send(c,(&respbuff + n*sizeof(char) ),buffsize-n,0)) > 0;)
            int a = 0;
           // for(n=0;(n += send(c, respbuff+a*sizeof(char) ,100,0)) > 0;a+=100) // good one
        for(n=0;(n += send(c, respbuff+a*sizeof(char) ,buffsize,0)) != 0;a+=n)
            ; // good one
          // while((n = send(c,respbuff,buffsize,0)) !=0)
            //for(n=0;(n = send(c, respbuff+a*sizeof(char) ,buffsize-a,0)) > 0; a+=n)
            //for(i=0; i< buffsize; i+=100)
           /* {
                n = send(c,respbuff,100,0);
                if(n == 0)
                {
                    exiterr("send error 0 bytes\n");
                }
                if(n == -1)
                {
                    exiterr("send error\n");
                }

                servlog("Bytes sent: %d\n", n);
                servlog("Sending info...\n");
            }*/

            servlog("Bytes sent: %d\n", n);

            break;
        }
        close(c);
        break;
    }

    //Finish Up
    close(sock);
    freeaddrinfo(servinfo); //Free address info
    exit(0);
}
