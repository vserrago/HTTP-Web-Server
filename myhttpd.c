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
unsigned char debugflag =0;         //Whether debug logging is enabled or not
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
        printf("Param: %s\n", argv[i]);
        if(argv[i][0] == '-')
        {
            int len = strlen(argv[i]);
            for(j=1; j<len; j++)
            {
                switch(argv[i][j])
                {
                    case 'p': 
                        printf("p param given\n");
                        portflag = 1;
                        break;
                    case 'd':
                        printf("d param given\n");
                        debugflag = 1;
                        break;
                    default:
                        printf("Unkown param given\n");
                        exit(0);
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
                printf("Portnumber is invalid\n");
                exit(0);
            }
            printf("Portnum is %s\n", port);
        }
        else
        {
            printf("Unkown arg given\n");
            exit(0);
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
        printf("getaddrinfo error: %s\n", gai_strerror(status));
        exit(0);
    }

    //Create socket
    sock = socket(servinfo->ai_family, servinfo->ai_socktype, servinfo->ai_protocol);
    if(sock == -1)
    {
        printf("Socket error\n");
        exit(0);
    }

    //Bind socket
    if(bind(sock, servinfo->ai_addr, servinfo->ai_addrlen) != 0)
    {
        printf("binderror\n");
        exit(0);
    }

    int backlog = 10; //Have a backlog of up to 10 requests

    //Listen on socket for incoming connections
    if(listen(sock, backlog) != 0)
    {
        printf("Listen error\n");
        exit(0);
    }

    socklen_t addr_size = sizeof socket_st;

    printf("Ready to recieve\n");
    for(;;) //Forever
    {
        int c = accept(sock, (struct sockaddr *) &socket_st, &addr_size);

        if(c < 0)
        {
            printf("Accept error\n");
            exit(0);
        }

        for(;;)
        {
            char  buff [1000];
            int bytesrecieved = recv(c,buff, 1000,0);

            if(bytesrecieved < 0)
            {
                printf("Recieve Error\n");
                exit (0);
            }

            if(bytesrecieved == 0)
            {
                printf("Connection Closed\n");
                exit(0);
            }

            printf("%d bytes recieved, Request:\n", bytesrecieved);
            printf("%s\n",buff);

            request* r = genreq();
            char* token;

            int n;

            //buff = "GET / HTTP/1.1";

            //Retrieve header info
            /*
            if((n = sscanf(buff, "%s %s %s", r->reqtype, r->reqfile, r->httpver)) != 3)
            {
                printf("scanf error, only %d values scanned in\n");
                printf("Reqtype: %s, Reqfile: %s, httpver: %s",  r->reqtype, r->reqfile, r->httpver);
                exit(0);
            }
            //*/
            
            /*
            for(i=0;i<3;i++)
            {
                char** saveptr;
                if((token = strtok_r(buff," ",saveptr)) == NULL)
                {
                    printf("Buff Token error\n");
                    exit(0);
                }

                switch (i)
                {
                    case 0: r->reqtype = cpynewstr(token); break;
                    case 1: r->reqfile = cpynewstr(token); break;
                    case 2: r->httpver = cpynewstr(token); break;
                }
            }
            */
            if((token = strtok(buff," ")) == NULL)
            {
                printf("Buff Token error\n");
                exit(0);
            }
            r->reqtype = cpynewstr(token); 

            if((token = strtok(NULL," ")) == NULL)
            {
                printf("Buff Token error\n");
                exit(0);
            }
            r->reqfile = cpynewstr(token); 

            if((token = strtok(NULL," ")) == NULL)
            {
                printf("Buff Token error\n");
                exit(0);
            }
            r->httpver = cpynewstr(token); 

            printf("Reqtype: %s, Reqfile: %s, httpver: %s\n",  r->reqtype, r->reqfile, r->httpver);


            /*
               printf("Message Recieved: %s", buff);
               if (send(c, "Hello, world!", 13, 0) == -1)
               perror("send");
               exit(0);//*/
        }
    }


    freeaddrinfo(servinfo); //Free address info
    exit(0);
}
