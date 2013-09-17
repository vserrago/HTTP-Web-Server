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

#define MINPORTNUM 60000
#define DEFAULTPORTNUM 61000

int main(int argc, char *argv [])
{
    int i, j;  //Loop Variables
    int sock;  //Socket file descriptor
    unsigned char portflag, debugflag; //Command line param flags
    int portnumber = DEFAULTPORTNUM; //Port Number, if given one.
    char * portnums = "";
    struct sockaddr_storage socket_st;
    int status;
    struct addrinfo hints;
    struct addrinfo *servinfo;

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
                        exit(1);
                        break;
                }
            }
        }
        else if(portflag)
        {
            portnumber = atoi(argv[i]);
            //Ensure that portnumber is in the range of useable ports
            if(portnumber < MINPORTNUM || USHRT_MAX < portnumber)
            {
                printf("Portnumber is invalid\n");
                exit(3);
            }
            printf("Portnum is %d\n", portnumber);
        }
        else
        {
            printf("Unkown arg given\n");
            exit(2);
        }
    }

    /* Socket Sutff */
    memset(&hints, 0, sizeof hints);    //Wipe space for hints
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;    //Use TCP socket
    hints.ai_flags = AI_PASSIVE;        //Figure out ip


    if ((status = getaddrinfo(NULL, "61000", &hints, &servinfo)) != 0)
    {
        printf("getaddrinfo error: %s\n", gai_strerror(status));
        exit(1);
    }

    sock = socket(servinfo->ai_family, servinfo->ai_socktype, servinfo->ai_protocol);
    if(sock == -1)
    {
        printf("Socket error\n");
        exit(0);
    }

    if(bind(sock, servinfo->ai_addr, servinfo->ai_addrlen) != 0)
    {
        printf("binderror\n");
        exit(0);
    }



    freeaddrinfo(servinfo); //Free address info
    exit(0);
}
